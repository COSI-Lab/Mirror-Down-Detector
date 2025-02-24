// Header Being Defined
#include <mirror/down_detector/bot.hpp>

// Standard Library Includes
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// Third Party Includes
#include <dpp/dpp.h>

// Project Includes
#include <mirror/down_detector/ping.hpp>
#include <mirror/down_detector/readFile.h>

void registerSlashCommands(dpp::cluster& bot, std::vector<std::string> envData)
{
    // /watch-mirror command
    dpp::slashcommand watchMirror(
        "watch-mirror",
        "Manage DownDetector channel",
        bot.me.id
    );
    watchMirror.add_option(
        dpp::command_option(
            dpp::co_sub_command,
            "add",
            "Add channel to DownDetecting list"
        )
            .add_option(dpp::command_option(
                dpp::co_role,
                "role",
                "Role to ping in channel (defaults to @everyone)",
                true
            ))
    );
    watchMirror.add_option(dpp::command_option(
        dpp::co_sub_command,
        "delete",
        "Delete a channel from DownDetecting List"
    ));

    // /ping command
    dpp::slashcommand pingCommand(
        "ping",
        "Ping an IP address or URL",
        bot.me.id
    );
    pingCommand.add_option(
        dpp::command_option(dpp::co_string, "address", "Address to ping", true)
    );

    if (envData[1] == "global")
    {
        // If we are running globally, register them for all guilds
        if (dpp::run_once<struct register_bot_commands>())
        {
            // Create commands
            bot.global_command_create(watchMirror);
            bot.global_command_create(pingCommand);
        }
    }
    else if (envData[1] == "test" && envData.size() >= 3)
    {
        // If we are running in test mode, register commands for the test guild
        long long guild_id = std::stoll(envData[2]);
        bot.guild_command_create(watchMirror, guild_id);
        bot.guild_command_create(pingCommand, guild_id);
    }
    else
    {
        throw std::invalid_argument("Invalid .env format");
    }
}

// TODO: break this function down a little bit
void watchMirrorCommand(
    dpp::interaction           interaction,
    const dpp::slashcommand_t& event,
    dpp::command_interaction   subcommand
)
{
    // Fetch the issuing user's permissions
    uint64_t          e_channel_id  = interaction.channel_id;
    dpp::user         e_user        = interaction.get_issuing_user();
    dpp::guild        e_guild       = interaction.get_guild();
    dpp::guild_member e_member      = e_guild.members.find(e_user.id)->second;
    dpp::permission   e_permissions = e_guild.base_permissions(e_member);

    // If the user can kick and ban members, we will consider them privileged
    // enough to use this command
    if (!e_permissions.can(dpp::p_kick_members, dpp::p_ban_members))
    {
        event.reply("You do not have permission to use this command.");
        return;
    }

    // TODO: make this pretty
    bool channelInFile = hasChannel(
        "/down-detector/resources/channels.txt",
        std::to_string(e_channel_id)
    );
    std::vector<std::vector<std::string>> channels_roles
        = readFile2d("/down-detector/resources/channels.txt");

    std::string subcommandName = subcommand.options[0].name;
    if (subcommandName == "add")
    { // /watch-mirror add
        dpp::role role = interaction.get_resolved_role(
            subcommand.options[0].get_value<dpp::snowflake>(0)
        );
        std::string roleMention = role.get_mention();

        if (!channelInFile)
        {
            channels_roles.push_back(std::vector<std::string> {
                std::to_string(e_channel_id),
                roleMention });
            writeFile2d(
                channels_roles,
                "/down-detector/resources/channels.txt"
            );
            event.reply("This channel now will recieve down-detection messages."
            );
        }
        else
        {
            event.reply("This channel already recieves down-detection messages."
            );
        }
    }
    else if (subcommandName == "delete")
    { // /watch-mirror delete
        if (!channelInFile)
        {
            event.reply("This channel is not recieving down-detection messages."
            );
            return;
        }

        int index = 0;
        for (int i = 0; i < channels_roles.size(); i++)
        {
            if (channels_roles[i][0] == std::to_string(e_channel_id))
            {
                index = i;
            }
        }

        channels_roles.erase(channels_roles.begin() + index);
        writeFile2d(channels_roles, "/down-detector/resources/channels.txt");
        event.reply(
            "This channel will no longer recieve down-detection messages."
        );
    }
}

void pingCommand(
    dpp::interaction           interaction,
    const dpp::slashcommand_t& event,
    dpp::command_interaction   subcommand
)
{
    event.reply("Pinging...");
    std::string address = std::get<std::string>(subcommand.options[0].value);
    std::pair<bool, std::string> pingObj = ping(address);
    if (pingObj.second != "")
    {
        // create embed
        dpp::embed embed;
        embed.set_title(address);
        embed.set_description(pingObj.second);
        // create embed footer
        dpp::embed_footer ef;
        embed.set_footer(
            ef.set_text("COSI Mirror Down Detection")
                .set_icon(
                    "https://avatars.githubusercontent.com/u/5634011?s=48&v=4"
                )
        );
        // create message object and set color based on ping success
        std::string messageContent;
        if (pingObj.first == 1)
        {
            messageContent = "Success.";
            embed.set_color(0x00000000
            ); // black hex with 2 FF in front for "alpha"
        }
        else
        {
            messageContent = "Failure.";
            embed.set_color(0xFFFF0000
            ); // red hex with 2 FF in front for "alpha"
        }
        dpp::message message(
            dpp::snowflake(interaction.channel_id),
            messageContent
        );
        // attach embed to message object
        message.add_embed(embed);
        // edit our response
        event.edit_response(message);
    }
    else
    {
        event.edit_response("invalid input.");
    }
}

void botThread(std::vector<std::string> envData)
{
    dpp::cluster bot(envData[0]);
    std::cout << "Key: " << envData[0] << "\n";

    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand(
        [](const dpp::slashcommand_t& event)
        {
            dpp::interaction         interaction = event.command;
            dpp::command_interaction cmd_data
                = interaction.get_command_interaction();

            if (interaction.get_command_name() == "watch-mirror")
            {
                watchMirrorCommand(interaction, event, cmd_data);
            }

            if (interaction.get_command_name()
                == "ping") // Fix param passing here
            {
                pingCommand(interaction, event, cmd_data);
            }
        }
    );

    bot.on_ready([&bot, &envData](const dpp::ready_t& event)
                 { registerSlashCommands(bot, envData); });

    bot.start(dpp::st_wait);
}

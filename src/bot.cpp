#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <dpp/dpp.h>

#include <ping.hpp>
#include <readFile.hpp>

void botThread(std::vector<std::string> envData)
{
    dpp::cluster bot(envData[0]);

    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand(
        [](const dpp::slashcommand_t& event)
        {
            // interaction and comd_data objects
            dpp::interaction         interaction = event.command;
            dpp::command_interaction cmd_data
                = interaction.get_command_interaction();

            // check if command is watch-mirror command
            if (interaction.get_command_name() == "watch-mirror")
            {
                // channel id
                uint64_t          e_channel_id = interaction.channel_id;
                // user object
                dpp::user         e_user       = interaction.get_issuing_user();
                // guild object
                dpp::guild        e_guild      = interaction.get_guild();
                // member object
                dpp::guild_member e_member
                    = e_guild.members.find(e_user.id)->second;
                // members permissions
                dpp::permission e_permissions
                    = e_guild.base_permissions(e_member);
                // bool representing mod status of the user
                bool is_mod = e_permissions.can(
                    dpp::p_kick_members,
                    dpp::p_ban_members
                );

                if (is_mod == true)
                {
                    // read the channel file into a list of channels and roles
                    std::vector<std::vector<std::string>> channels_roles
                        = readFile2d("../channels.txt");

                    // check to see if the channel is in the file to avoid
                    // duplicates
                    bool channelInFile = false;
                    for (int i = 0; i < channels_roles.size(); i++)
                    {
                        if (std::to_string(e_channel_id)
                            == channels_roles[i][0])
                        {
                            channelInFile = true;
                        }
                    }

                    // Get the sub command
                    dpp::command_data_option subcommand = cmd_data.options[0];
                    // Check if the subcommand is "add"
                    if (subcommand.name == "add")
                    {
                        // Get the role from the parameter
                        dpp::role role = interaction.get_resolved_role(
                            subcommand.get_value<dpp::snowflake>(0)
                        );
                        std::string roleMention = role.get_mention();

                        // add channel to file if not already there
                        if (channelInFile == false)
                        {
                            // add channel role pair to channel_roles
                            channels_roles.push_back(std::vector<std::string> {
                                std::to_string(e_channel_id),
                                roleMention });
                            // write channel role vector of vectors to file
                            writeFile2d(channels_roles, "../channels.txt");

                            event.reply("this channel will now recieve "
                                        "down-detection messages.");
                        }
                        else
                        {
                            event.reply("this channel is already recieving "
                                        "down-detection messages.");
                        }
                    }
                    // Check if the subcommand is "delete"
                    if (subcommand.name == "delete")
                    {
                        // dont attempt to delete if channel not there
                        if (channelInFile == true)
                        {
                            // remove channel from vector
                            int index = 0;
                            for (int i = 0; i < channels_roles.size(); i++)
                            {
                                if (channels_roles[i][0]
                                    == std::to_string(e_channel_id))
                                {
                                    index = i;
                                }
                            }
                            channels_roles.erase(
                                channels_roles.begin() + index
                            );

                            // write channel role vector of vectors to file
                            writeFile2d(channels_roles, "../channels.txt");

                            event.reply("this channel will no longer recieve "
                                        "down-detection messages.");
                        }
                        else
                        {
                            event.reply("this channel is not recieving "
                                        "down-detection messages");
                        }
                    }
                }
                else
                {
                    event.reply("You dont have permission to use that command."
                    );
                }
            }

            if (interaction.get_command_name() == "ping")
            {
                // gets the input string from user from the command
                std::string address = cmd_data.get_value<std::string>(0);
                // placeholder response to prevent timeout
                event.reply("pinging...");
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
                            .set_icon("https://avatars.githubusercontent.com/u/"
                                      "5634011?s=48&v=4")
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
        }
    );

    bot.on_ready(
        [&bot, &envData](const dpp::ready_t& event)
        {
            // create our watchMirror slash command object and then add
            // subcommands and options
            dpp::slashcommand watchMirror(
                "watch-mirror",
                "Manage DownDetector channel",
                bot.me.id
            );
            watchMirror.add_option(
                // Create a subcommand type option for add.
                dpp::command_option(
                    dpp::co_sub_command,
                    "add",
                    "Add channel to DownDetecting list"
                )
                    .add_option(dpp::command_option(
                        dpp::co_role,
                        "role",
                        "role to ping in channel",
                        true
                    ))
            );
            watchMirror.add_option(
                // Create another subcommand type option for delete.
                dpp::command_option(
                    dpp::co_sub_command,
                    "delete",
                    "Delete a channel from DownDetecting List"
                )
            );

            // create ping slach command object
            dpp::slashcommand pingCommand(
                "ping",
                "ping an ip address or url",
                bot.me.id
            );
            pingCommand.add_option(dpp::command_option(
                dpp::co_string,
                "address",
                "address to ping",
                true
            ));

            // register slash commands globably using run_once so that it only
            // runs once
            if (envData[1] == "global")
            {
                if (dpp::run_once<struct register_bot_commands>())
                {
                    // Create commands
                    bot.global_command_create(watchMirror);
                    bot.global_command_create(pingCommand);
                }
            }
            // register slash commands for a specific server for testing
            else if (envData[1] == "test" && envData.size() >= 3)
            {
                // get guild id for test server
                long long guild_id
                    = std::stoll(envData[2]); // stoll is string to long long
                // Create commands
                bot.guild_command_create(watchMirror, guild_id);
                bot.guild_command_create(pingCommand, guild_id);
            }
            else
            {
                throw std::invalid_argument("incorrect env file");
            }
        }
    );

    bot.start(dpp::st_wait);
}

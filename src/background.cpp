#include <ctime>
#include <future>
#include <iostream>
#include <string>
#include <unistd.h> //linux only library
#include <utility>
#include <vector>

#include <dpp/dpp.h>

#include "http.h"
#include "ping.h"
#include "readFile.h"

const int PING_DELAY = 900;
const int POLL_DELAY = 10;

dpp::embed
createErrorEmbed(std::vector<std::pair<uint16_t, std::string>> errorCodes)
{
    // create embed object
    dpp::embed embed;

    if (errorCodes[0].first == 200 && errorCodes[1].first == 200
        && errorCodes[2].first == 1)
    {
        // everything is back to running well
        embed.set_title("Mirror Is Back Up!!!!");
        embed.set_color(0x00000000); // black hex with 2 FF in front for "alpha"
    }
    else
    {
        // something is down
        embed.set_title("Mirror Is Down!!!!");
        embed.set_color(0xFFFF0000); // red hex with 2 FF in front for "alpha"
    }

    // create a field for /status page
    embed.add_field(
        "mirror.clarkson.edu/status",
        std::string("HTTP status: ") + std::to_string(errorCodes[0].first)
    );
    // create a field for /home page
    embed.add_field(
        "mirror.clarkson.edu/home",
        std::string("HTTP status: ") + std::to_string(errorCodes[1].first)
    );
    embed.set_description(errorCodes[2].second);
    dpp::embed_footer ef;
    embed.set_footer(
        ef.set_text("COSI Mirror Down Detection")
            .set_icon("https://avatars.githubusercontent.com/u/5634011?s=48&v=4"
            )
    );
    return embed;
}

/**
 * Returns a vector of pairs where each pair is in the format of
 * (status code, status message)
 */
std::vector<std::pair<uint16_t, std::string>>
checkMirrorStatus(dpp::cluster& bot)
{
    std::promise<uint16_t> promiseStatus, promiseHome;

    std::future<uint16_t> futureStatus = promiseStatus.get_future();
    std::future<uint16_t> futureHome   = promiseHome.get_future();

    // check /status
    request(
        "https://mirror.clarkson.edu/status",
        [&promiseStatus](long resp) { promiseStatus.set_value(resp); }
    );

    // check /home
    request(
        "https://mirror.clarkson.edu/home",
        [&promiseHome](long resp) { promiseHome.set_value(resp); }
    );

    // Ping function returns true if the address responds to a ping
    std::future<std::pair<bool, std::string>> pingResponse
        = std::async(ping, "mirror.clarkson.edu");

    // wait for futures to complete before accessing the result
    std::pair<bool, std::string> pingObj   = pingResponse.get(); // blocking
    uint16_t                     statusInt = futureStatus.get(); // blocking
    uint16_t                     homeInt   = futureHome.get();   // blocking
    std::vector<std::pair<uint16_t, std::string>> status
        = { std::make_pair(statusInt, ""), // our HTTP requests have no message,
                                           // so feed the pair an empty string
            std::make_pair(homeInt, ""),
            std::make_pair((uint16_t)pingObj.first, pingObj.second) };
    return status;
}

/**
 * This function decides whether or not the bot needs to send an alert message.
 * This function might not be necessary -- there used to be a complicated
 * state machine to work this out.
 *
 * Any time the status changes, we want to send a message.
 */
bool handleMessageConditions(
    std::vector<std::pair<uint16_t, std::string>> prevStatus,
    std::vector<std::pair<uint16_t, std::string>> currentStatus
)
{
    if (prevStatus.size() == 0)
    { // First loop condition
        return false;
    }

    for (int i = 0; i < currentStatus.size(); i++)
    {
        if (prevStatus[i].first != currentStatus[i].first)
        {
            return true;
        }
    }
    return false;
}

time_t lastPing = -1;

void sendEmbed(
    dpp::cluster&                                 bot,
    std::vector<std::pair<uint16_t, std::string>> status
)
{
    bool discordPing = false;
    if (lastPing == -1)
    {
        discordPing = true;
    }
    if (lastPing + PING_DELAY <= std::time(nullptr))
    {
        discordPing = true;
    }

    std::vector<std::vector<std::string>> channels_roles
        = readFile2d("../channels.txt");

    for (int i = 0; i < channels_roles.size(); i++)
    {
        std::time_t timestamp = std::time(nullptr);

        long long channel_id = std::stoll(channels_roles[i][0]);

        // The ping role is stored in channels.txt after the channel ID
        std::string role_mention = "";
        if (discordPing)
        {
            role_mention = channels_roles[i][1] + " ";
            time(&lastPing);
        }

        dpp::message message(
            dpp::snowflake(channel_id),
            std::string(role_mention) + "<t:" + std::to_string(timestamp)
                + ":F>"
        );

        if (discordPing)
        {
            message.set_allowed_mentions(
                false,
                true,
                false,
                false,
                std::vector<dpp::snowflake> {},
                std::vector<dpp::snowflake> {}
            );
        }

        dpp::embed embed = createErrorEmbed(status);
        message.add_embed(embed);

        bot.message_create(
            message,
            [&bot](const dpp::confirmation_callback_t& callback)
            {
                if (callback.is_error())
                {
                    std::cerr << "Failed to send message" << std::endl;
                }
            }
        );
    }
}

void backgroundThread(std::vector<std::string> envData)
{
    dpp::cluster bot { envData[0] };
    bot.on_log(dpp::utility::cout_logger());
    bot.on_ready(
        [&bot, &envData](const dpp::ready_t& event)
        {
            std::vector<std::pair<uint16_t, std::string>> prevStatus {};
            while (true)
            {
                std::vector<std::pair<uint16_t, std::string>> currentStatus
                    = checkMirrorStatus(bot);
                bool sendMessage
                    = handleMessageConditions(prevStatus, currentStatus);
                if (sendMessage)
                {
                    sendEmbed(bot, currentStatus);
                }

                prevStatus = currentStatus;

                sleep(POLL_DELAY);
            }
        }
    );
    bot.start(dpp::st_wait);
}
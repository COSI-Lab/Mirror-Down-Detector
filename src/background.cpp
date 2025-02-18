#include <ctime>
#include <future>
#include <iostream>
#include <string>
#include <unistd.h> //linux only library
#include <utility>
#include <vector>

#include <dpp/dpp.h>

#include <ping.hpp>
#include <readFile.hpp>

dpp::embed createErrorEmbed(std::vector<uint16_t> errorCodes, std::string ping)
{
    // create embed object
    dpp::embed embed;

    if (errorCodes[0] == 200 && errorCodes[1] == 200 && errorCodes[2] == 1)
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
        std::string("HTTP status: ") + std::to_string(errorCodes[0])
    );
    // create a field for /home page
    embed.add_field(
        "mirror.clarkson.edu/home",
        std::string("HTTP status: ") + std::to_string(errorCodes[1])
    );
    // set the discription equal to the ping text
    embed.set_description(ping);
    // create footer
    dpp::embed_footer ef;
    embed.set_footer(
        ef.set_text("COSI Mirror Down Detection")
            .set_icon("https://avatars.githubusercontent.com/u/5634011?s=48&v=4"
            )
    );
    return embed;
}

void backgroundThread(std::vector<std::string> envData)
{
    dpp::cluster bot(envData[0]);

    bot.on_log(dpp::utility::cout_logger());

    bot.on_ready(
        [&bot, &envData](const dpp::ready_t& event)
        {
            uint16_t              state = 0;
            std::vector<uint16_t> errorCodes { 200, 200, 1 };

            std::time_t most_recient_ping = -1;

            while (true)
            {
                // sleep for set time in seconds
                sleep(60);

                // we use promises to get data out of the callback functions
                // from bot.request
                std::promise<uint16_t> promiseStatus;
                std::promise<uint16_t> promiseHome;
                // we use futures to wait for our promises to resolve
                std::future<uint16_t> futureStatus = promiseStatus.get_future();
                std::future<uint16_t> futureHome   = promiseHome.get_future();

                // bot.request is an asynchronous function
                //  check /status
                bot.request(
                    "https://mirror.clarkson.edu/status",
                    dpp::m_get,
                    [&promiseStatus](
                        const dpp::http_request_completion_t& request
                    ) { promiseStatus.set_value(request.status); }
                );
                // check /home
                bot.request(
                    "https://mirror.clarkson.edu/home",
                    dpp::m_get,
                    [&promiseHome](const dpp::http_request_completion_t& request
                    ) { promiseHome.set_value(request.status); }
                );

                // asycronosly ping mirror while http requests are running
                std::future<std::pair<bool, std::string>> futurePingObj
                    = std::async(ping, "mirror.clarkson.edu");

                // wait for futures to complete before accessing the result
                std::pair<bool, std::string> pingObj
                    = futurePingObj.get();               // blocking
                uint16_t statusInt = futureStatus.get(); // blocking
                uint16_t homeInt   = futureHome.get();   // blocking

                // create currient error codes object
                std::vector<uint16_t> currientErrorCodes
                    = { statusInt, homeInt, pingObj.first };

                // handle the state machine
                bool sendMessage = false;
                switch (state)
                {
                case 0:
                case 1:
                    if (currientErrorCodes[0] == 200
                        && currientErrorCodes[1] == 200
                        && currientErrorCodes[2] == 1)
                    {
                        state = 0;
                    }
                    else
                    {
                        state++;
                        if (state == 2)
                        {
                            sendMessage = true;
                        }
                    }
                    break;
                case 2:
                    if (currientErrorCodes[0] == 200
                        && currientErrorCodes[1] == 200
                        && currientErrorCodes[2] == 1)
                    {
                        state       = 0;
                        sendMessage = true;
                    }
                    else
                    {
                        for (uint16_t i = 0; i < errorCodes.size(); i++)
                        {
                            if (errorCodes[i] != currientErrorCodes[i])
                            {
                                sendMessage = true;
                            }
                        }
                    }
                }

                if (sendMessage == true)
                {
                    // update error codes
                    errorCodes = currientErrorCodes;

                    // check last discord ping timestamp to see if we should
                    // ping on discord with the message to prevent ping spam
                    bool discordPing = false;
                    if (most_recient_ping == -1
                        || most_recient_ping + 900 <= std::time(0))
                    { // if 15 minutes have passed since the last discordping
                      // (or its the first ping)
                        most_recient_ping = std::time(0);
                        discordPing       = true;
                    }

                    // read channel file
                    std::vector<std::vector<std::string>> channels_roles
                        = readFile2d("../channels.txt");

                    // send messages to each channel in file
                    for (int i = 0; i < channels_roles.size(); i++)
                    {
                        // currient timestamp
                        std::time_t timestamp = std::time(nullptr);
                        // convet channel id to long long
                        long long channel_id = std::stoll(channels_roles[i][0]);
                        // convet role ping to string
                        std::string role_mention = "";
                        if (discordPing == true)
                        {
                            role_mention = channels_roles[i][1] + " ";
                        }
                        // create a message object
                        dpp::message message(
                            dpp::snowflake(channel_id),
                            std::string(role_mention)
                                + "<t:" + std::to_string(timestamp) + ":F>"
                        );
                        // make the role pingable in the message
                        if (discordPing == true)
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
                        // create an embed object
                        dpp::embed embed = createErrorEmbed(
                            currientErrorCodes,
                            pingObj.second
                        );
                        // add embed object to message object
                        message.add_embed(embed);

                        // send message
                        bot.message_create(
                            message,
                            [&bot](const dpp::confirmation_callback_t& callback)
                            {
                                if (callback.is_error())
                                {
                                    // log error
                                    std::cout << "failed to send message"
                                              << std::endl;
                                }
                            }
                        );
                    }
                }
            }
        }
    );
    bot.start(dpp::st_wait);
}

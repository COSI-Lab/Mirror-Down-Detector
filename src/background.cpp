// Header Being Defined
#include <mirror/down_detector/background.hpp>

// System Includes
#include <unistd.h>

// Standard Library Includes
#include <algorithm>
#include <cstdint>
#include <ctime>
#include <future>
#include <iostream>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

// Third Party Includes
#include <dpp/dpp.h>
#include <spdlog/spdlog.h>

// Project Includes
#include <mirror/down_detector/http.hpp>
#include <mirror/down_detector/ping.hpp>
#include <mirror/down_detector/readFile.hpp>

constexpr std::uint32_t PING_DELAY = 900;
constexpr std::uint32_t POLL_DELAY = 10;

dpp::embed
createErrorEmbed(std::vector<std::pair<std::uint16_t, std::string>> errorCodes)
{
    // create embed object
    dpp::embed embed;

    if (errorCodes.at(0).first == 200 && errorCodes.at(1).first == 200
        && errorCodes.at(2).first == 1)
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
        std::string("HTTP status: ") + std::to_string(errorCodes.at(0).first)
    );
    // create a field for /home page
    embed.add_field(
        "mirror.clarkson.edu/home",
        std::string("HTTP status: ") + std::to_string(errorCodes.at(1).first)
    );
    embed.set_description(errorCodes.at(2).second);
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
 * This return type could probably use a typedef sometime in the future
 */
std::vector<std::pair<uint16_t, std::string>>
checkMirrorStatus(dpp::cluster& bot)
{
    std::promise<std::uint16_t> promiseStatus;
    std::promise<std::uint16_t> promiseHome;

    auto futureStatus = promiseStatus.get_future();
    auto futureHome   = promiseHome.get_future();

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
    std::uint16_t                statusInt = futureStatus.get(); // blocking
    std::uint16_t                homeInt   = futureHome.get();   // blocking
    std::vector<std::pair<uint16_t, std::string>> status
        = { std::make_pair(statusInt, ""), // our HTTP requests have no message,
                                           // so feed the pair an empty string
            std::make_pair(homeInt, ""),
            std::make_pair(
                static_cast<std::uint16_t>(pingObj.first),
                pingObj.second
            ) };
    return status;
}

/**
 * This function decides whether or not the bot needs to send an alert message.
 * This function might not be necessary -- there used to be a complicated
 * state machine to work this out.
 *
 * Any time the status changes, we want to send a message.
 */
auto statusHasChanged(
    const std::ranges::viewable_range auto& prevStatus,
    const std::ranges::viewable_range auto& currentStatus
) -> bool
{
    return std::ranges::any_of(
        std::views::zip(prevStatus, currentStatus),
        [](const auto& item) -> bool
        {
            const auto [prev, curr] = item;
            return prev.first != curr.first;
        }
    );
}

::time_t lastPing = -1;

void sendUpdatedStatus(
    dpp::cluster&                                 bot,
    std::vector<std::pair<uint16_t, std::string>> status
)
{
    bool shouldIncludePing = false;
    if (lastPing == -1)
    {
        shouldIncludePing = true;
    }
    if (lastPing + PING_DELAY <= std::time(nullptr))
    {
        shouldIncludePing = true;
    }

    // !: We shouldn't re-read this file every time we send an embed.
    // TODO: Store contents of file in memory
    std::vector<std::vector<std::string>> channels_roles
        = readFile2d("/down-detector/resources/channels.txt");

    for (std::size_t idx = 0; idx < channels_roles.size(); idx++)
    {
        std::time_t timestamp = std::time(nullptr);

        long long channel_id = std::stoll(channels_roles.at(idx).at(0));

        // The ping role is stored in channels.txt after the channel ID
        std::string role_mention = "";
        if (shouldIncludePing)
        {
            role_mention = channels_roles.at(idx).at(1) + " ";
            std::time(&lastPing);
        }

        dpp::message message(
            dpp::snowflake(channel_id),
            std::string(role_mention) + "<t:" + std::to_string(timestamp)
                + ":F>"
        );

        if (shouldIncludePing)
        {
            constexpr bool                        allowUserMentions     = false;
            constexpr bool                        allowRoleMentions     = true;
            constexpr bool                        allowEveryoneMentions = true;
            constexpr bool                        isReplyMessage        = false;
            constexpr std::vector<dpp::snowflake> allowedUsers          = {};
            constexpr std::vector<dpp::snowflake> allowedRoles          = {};

            message.set_allowed_mentions(
                allowUserMentions,
                allowRoleMentions,
                allowEveryoneMentions,
                isReplyMessage,
                allowedUsers,
                allowedRoles
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
                    spdlog::error("Failed to send message!");
                }
            }
        );
    }
}

auto backgroundThread(const std::vector<std::string>& envData) -> void
{
    dpp::cluster bot { envData.at(0) };
    bot.on_log(dpp::utility::cout_logger());
    bot.on_ready(
        [&bot, &envData](const dpp::ready_t& event)
        {
            std::vector<std::pair<std::uint16_t, std::string>> prevStatus {};
            while (true)
            {
                auto currentStatus = checkMirrorStatus(bot);
                if (prevStatus.empty()
                    || statusHasChanged(prevStatus, currentStatus))
                {
                    sendUpdatedStatus(bot, currentStatus);
                }

                prevStatus = currentStatus;

                ::sleep(POLL_DELAY);
            }
        }
    );
    bot.start(dpp::st_wait);
}

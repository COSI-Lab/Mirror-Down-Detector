# DownDetector bot

To use the bot add the bot to your server with this [Link](https://discord.com/oauth2/authorize?client_id=1159278212099088425&permissions=158720&scope=bot)

## Slash commands
/watch-mirror add {@role}
Adds the channel the comand is run in to the list of channels watching mirror.
The role passed in with the command is piged by the bot when there is a status update.

/watch-mirror delete
Deletes the channel the command is run in from the watch list.

/ping {address}
used to ping any hostname or ip address.
# Development Notes

## Dependencies

Install D++ using instructions on the D++ Documentation Website.
[Installation Instructions](https://dpp.dev/installing.html)

There are many ways to install I personally built D++ from source from the github.
[D++ Github](https://github.com/brainboxdotcc/DPP)

## Development ENV File

Create a .env file with the following 3 lines of text for registering slash commands in a single server for testing.

```bash
DiscordToken
test
guild_id
```

## Production ENV File

Create a .env file with the following 2 lines of text for registering slash commands globally for any server the bot joins.

```bash
DiscordToken
global
```

### Start the bot

To start the bot just run `docker-compose up`.
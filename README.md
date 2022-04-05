# Media/Attachment Moderation Bot

This bot will censor any messages with attachments until approved by a mod

This repo assumes that you have D++ installed

## Compilation 

    mkdir build
    cd build
    cmake ..
    make -j

If DPP is installed in a different location you can specify the root directory to look in while running cmake 

    cmake .. -DDPP_ROOT_DIR=<your-path>

## Configuring the bot

Edit the default config.json file
```json
{
	"token": "your bot token here"
	"modChannelID":Media moderation channel ID here
}
```

Start the bot:

    cd build
    ./crektlays


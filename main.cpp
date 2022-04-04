#include <dpp/dpp.h>
#include <dpp/message.h>
#include <sstream>

const std::string BOT_TOKEN = "";
const dpp::snowflake SERVER_ID = 959613365678923794;
const dpp::snowflake MOD_CHANNEL_ID = 960215514402127873;

int main()
{
	dpp::cluster bot(BOT_TOKEN);

	bot.on_log(dpp::utility::cout_logger());

	bot.intents |= dpp::intents::i_message_content;

	std::vector<dpp::message*> heldMessages;
	std::map<dpp::message*,std::vector<std::pair<std::string,std::string>>> msgFiles;

	// callback for whenever a message gets sent
	bot.on_message_create(
			[&] (const dpp::message_create_t& event)
			{
				// if there are attachments and author is not a bot
				if(!event.msg.attachments.empty() && !event.msg.author.is_bot())
				{
					// Stick author and message contents into bot's message
					std::ostringstream contents;
					contents << dpp::utility::timestamp(time(0),dpp::utility::time_format::tf_short_datetime) << '\n'
					<< "``Author: ``" << event.msg.author.get_mention() << '\n'
					<< "``Message Body:\n" << event.msg.content << "``\n";

					dpp::message* msg = new dpp::message(MOD_CHANNEL_ID,contents.str());

					// loop through all attachments
					for(int i=0;i<event.msg.attachments.size();i++)
					{
						bool bWaitUp = true;
						std::string url = event.msg.attachments[i].url;

						// get filename from url
						std::string filename = url.substr(url.find_last_of('/')+1);

						// callback for when attachment is downloaded
						bot.request(url, dpp::m_get, 
						[&msg,&filename,&bWaitUp,&msgFiles]
						(const dpp::http_request_completion_t& httpRequestCompletion)
						{
							if(httpRequestCompletion.status == 200)
							{
								// add attachment to bot message
								msg->add_file(filename, httpRequestCompletion.body);
								std::pair<std::string,std::string> file(filename,httpRequestCompletion.body);
								msgFiles[msg].push_back(file);
							}
							// allow the loop to continue to the next attachment
							bWaitUp=false;
						}
						);
						
						// wait for attachment to finish downloading before progressing
						while(bWaitUp)
						{}
					}

					msg->add_component(dpp::component().add_component(
								dpp::component().set_label("Approve").
								set_type(dpp::cot_button).
								set_emoji(u8"✔️").
								set_style(dpp::cos_success).
								set_id('a'+std::to_string(heldMessages.size())+'|'+std::to_string(event.msg.channel_id))
								).
								add_component(dpp::component().set_label("Deny").
								set_type(dpp::cot_button).
								set_emoji(u8"✖️").
								set_style(dpp::cos_danger).
								set_id('d'+std::to_string(heldMessages.size()))));
					bot.message_create(*msg);
					delete msg;

					// delete original message
					bot.message_delete(event.msg.id,event.msg.channel_id);
				}
				// if the msg is from bot with attachments
				else if(event.msg.attachments.size() != 0 && event.msg.author.is_bot())
				{
					// add message to vector
					dpp::message* msg = new dpp::message();
					*msg = event.msg;
					heldMessages.push_back(msg);
				}
			}
			);
	
	bot.on_button_click([&] (const dpp::button_click_t& event)
			{
				std::string button = (event.custom_id[0] == 'a') ? ("approved") : ("denied");

				int idx = std::stoi(event.custom_id.substr(1,event.custom_id.find('|')));
				dpp::message* msg = heldMessages[idx];

				bool bCleanupTime = false;

				if(button=="approved")
				{
					dpp::snowflake channel = std::stoll(event.custom_id.substr(event.custom_id.find('|')+1));
					dpp::message approvedMsg(channel,msg->content);
					for(auto file : msgFiles[msg])
					{
						approvedMsg.add_file(file.first,file.second);
					}
					bot.message_create(approvedMsg,[&bCleanupTime](const dpp::confirmation_callback_t& c)
							{
								bCleanupTime = true;
							}
							);
				}
				else bCleanupTime = true;

				// wait until we have sent if approved
				while(!bCleanupTime)
				{}

				// delete files
				msgFiles[msg].clear();
				msgFiles.erase(msg);

				// delete held message
				bot.message_delete(msg->id,MOD_CHANNEL_ID);
				delete msg;
				heldMessages.erase(heldMessages.begin() + idx);

				dpp::message response(MOD_CHANNEL_ID,"message " + button);
				event.reply("");
			}
			);

	bot.start(false);
}

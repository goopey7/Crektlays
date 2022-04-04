#include <dpp/dpp.h>
#include <dpp/message.h>
#include <sstream>

const std::string BOT_TOKEN = "OTU5NjA4NjE1OTczNTY0NDg2.YkeXKg.QNldwVxn09sMMlGF8LSzXOVx76k";
const dpp::snowflake SERVER_ID = 959613365678923794;
const dpp::snowflake MOD_CHANNEL_ID = 960215514402127873;

int main()
{
	dpp::cluster bot(BOT_TOKEN);

	bot.on_log(dpp::utility::cout_logger());

	bot.intents |= dpp::intents::i_message_content;

	std::vector<dpp::message*> heldMessages;

	// callback for whenever a message gets sent
	bot.on_message_create(
			[&] (const dpp::message_create_t& event)
			{
				// if there are attachments and author is not a bot
				if(!event.msg.attachments.empty() && !event.msg.author.is_bot())
				{
					// Stick author and message contents into bot's message
					std::ostringstream contents;
					contents << "``Author: ``" << event.msg.author.get_mention() << '\n'
					<< "``Message Body:\n" << event.msg.content << "``\n";

					dpp::message* msg = new dpp::message(MOD_CHANNEL_ID,contents.str());
					heldMessages.push_back(msg);

					// loop through all attachments
					for(int i=0;i<event.msg.attachments.size();i++)
					{
						bool bWaitUp = true;
						std::string url = event.msg.attachments[i].url;

						// get filename from url
						std::string filename = url.substr(url.find_last_of('/')+1);

						// callback for when attachment is downloaded
						bot.request(url, dpp::m_get, 
						[&msg,&filename,&bWaitUp]
						(const dpp::http_request_completion_t& httpRequestCompletion)
						{
							if(httpRequestCompletion.status == 200)
							{
								// add attachment to bot message
								msg->add_file(filename, httpRequestCompletion.body);
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
								set_id("approved")
								).
								add_component(dpp::component().set_label("Deny").
								set_type(dpp::cot_button).
								set_emoji(u8"✖️").
								set_style(dpp::cos_danger).
								set_id("denied")));
					bot.message_create(*msg);

					// delete original message
					bot.message_delete(event.msg.id,event.msg.channel_id);
				}
			}
			);
	
	bot.on_button_click([&] (const dpp::button_click_t& event)
			{
			}
			);

	bot.start(false);
}

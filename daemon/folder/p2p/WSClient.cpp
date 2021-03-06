/* Copyright (C) 2015 Alexander Shishenko <GamePad64@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "WSClient.h"
#include "Client.h"
#include "P2PFolder.h"
#include "folder/FolderGroup.h"

namespace librevault {

WSClient::WSClient(Client& client, P2PProvider& provider) : WSService(client, provider) {
	/* WebSockets client initialization */
	// General parameters
	ws_client_.init_asio(&client_.network_ios());
	ws_client_.set_user_agent(Version::current().user_agent());
	ws_client_.set_max_message_size(10 * 1024 * 1024);

	// Handlers
	ws_client_.set_tcp_pre_init_handler(std::bind(&WSClient::on_tcp_pre_init, this, std::placeholders::_1, connection::CLIENT));
	ws_client_.set_tls_init_handler(std::bind(&WSClient::on_tls_init, this, std::placeholders::_1));
	ws_client_.set_tcp_post_init_handler(std::bind(&WSClient::on_tcp_post_init, this, std::placeholders::_1));
	ws_client_.set_open_handler(std::bind(&WSClient::on_open, this, std::placeholders::_1));
	ws_client_.set_message_handler(std::bind(&WSClient::on_message_internal, this, std::placeholders::_1, std::placeholders::_2));
	ws_client_.set_fail_handler(std::bind(&WSClient::on_disconnect, this, std::placeholders::_1));
	ws_client_.set_close_handler(std::bind(&WSClient::on_disconnect, this, std::placeholders::_1));
}

void WSClient::connect(url node_url, std::shared_ptr<FolderGroup> group_ptr) {
	log_->trace() << log_tag() << BOOST_CURRENT_FUNCTION << " " << (std::string)node_url;
	websocketpp::lib::error_code ec;

	// Building url
	node_url.scheme = "wss";
	node_url.query = "/";
	node_url.query += dir_hash_to_query(group_ptr->hash());

	auto connection_ptr = ws_client_.get_connection(node_url, ec);
	if(ec) {
		log_->warn() << log_tag() << "Error connecting to " << (std::string)node_url;
		return;
	}

	connection& conn = ws_assignment_[websocketpp::connection_hdl(connection_ptr)];
	conn.hash = group_ptr->hash();

	log_->debug() << log_tag() << "Added node " << std::string(node_url);

	ws_client_.connect(connection_ptr);
}

void WSClient::connect(const tcp_endpoint& node_endpoint, std::shared_ptr<FolderGroup> group_ptr) {
	if(!group_ptr->have_p2p_dir(node_endpoint) && !provider_.is_loopback(node_endpoint)) {
		url node_url;

		if(node_endpoint.address().is_v6()) node_url.is_ipv6 = true;

		node_url.host = node_endpoint.address().to_string();
		node_url.port = node_endpoint.port();

		connect(node_url, group_ptr);
	}
}

void WSClient::connect(const tcp_endpoint& node_endpoint, const blob& pubkey, std::shared_ptr<FolderGroup> group_ptr) {
	if(!group_ptr->have_p2p_dir(pubkey) && !provider_.is_loopback(pubkey)){
		connect(node_endpoint, group_ptr);
	}
}

void WSClient::on_message_internal(websocketpp::connection_hdl hdl, client::message_ptr message_ptr) {
	on_message(hdl, message_ptr->get_payload());
}

std::string WSClient::dir_hash_to_query(const blob& dir_hash) {
	return crypto::Hex().to_string(dir_hash);
}

void WSClient::send_message(websocketpp::connection_hdl hdl, const blob& message) {
	log_->trace() << log_tag() << BOOST_CURRENT_FUNCTION;
	ws_client_.get_con_from_hdl(hdl)->send(message.data(), message.size());
}

std::string WSClient::errmsg(websocketpp::connection_hdl hdl) {
	return ws_client_.get_con_from_hdl(hdl)->get_transport_ec().message();
}

} /* namespace librevault */

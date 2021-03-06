/* Copyright (C) 2016 Alexander Shishenko <GamePad64@gmail.com>
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
#pragma once
#include "pch.h"
#include "util/parse_url.h"
#include <json/json.h>
#include <librevault/Meta.h>
#include <librevault/Secret.h>

namespace librevault {

struct FolderParams {
	FolderParams(){}
	FolderParams(const Json::Value& json_params) {
		FolderParams defaults;

		// Necessary
		secret = json_params["secret"].asString();
		path = json_params["path"].asString();

		// Optional
		system_path = json_params.get("system_path", (path / ".librevault").string()).asString();
		index_event_timeout = std::chrono::milliseconds(json_params.get("index_event_timeout", Json::Value::UInt64(defaults.index_event_timeout.count())).asUInt64());
		preserve_unix_attrib = json_params.get("preserve_unix_attrib", defaults.preserve_unix_attrib).asBool();
		preserve_windows_attrib = json_params.get("preserve_windows_attrib", defaults.preserve_windows_attrib).asBool();
		preserve_symlinks = json_params.get("preserve_symlinks", defaults.preserve_symlinks).asBool();
		normalize_unicode = json_params.get("normalize_unicode", defaults.normalize_unicode).asBool();
		chunk_strong_hash_type = Meta::StrongHashType(json_params.get("chunk_strong_hash_type", defaults.chunk_strong_hash_type).asUInt());
		full_rescan_interval = std::chrono::seconds(json_params.get("full_rescan_interval", Json::Value::UInt64(defaults.full_rescan_interval.count())).asUInt64());

		for(auto ignore_path : json_params["ignore_paths"])
			ignore_paths.push_back(ignore_path.asString());
		for(auto node : json_params["nodes"])
			nodes.push_back(node.asString());
	}

	/* Parameters */
	Secret secret;
	fs::path path;
	fs::path system_path;
	std::chrono::milliseconds index_event_timeout = std::chrono::milliseconds(1000);
	bool preserve_unix_attrib = false;
	bool preserve_windows_attrib = false;
	bool preserve_symlinks = true;
	bool normalize_unicode = true;
	Meta::StrongHashType chunk_strong_hash_type = Meta::StrongHashType::SHA3_224;
	std::chrono::seconds full_rescan_interval = std::chrono::seconds(600);
	std::vector<std::string> ignore_paths;
	std::vector<url> nodes;
};

} /* namespace librevault */

/* Copyright (C) 2015 Alexander Shishenko <GamePad64@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "../../pch.h"
#pragma once

namespace librevault {

class Session;
class NodeKey {
public:
	using private_key_t = CryptoPP::DL_PrivateKey_EC<CryptoPP::ECP>;

	NodeKey(Session& session);
	virtual ~NodeKey();

	const private_key_t& private_key() const {return private_key_;}
	const blob& public_key() const {return public_key_;}

private:
	Session& session_;
	std::shared_ptr<spdlog::logger> log_;

	private_key_t private_key_;
	blob public_key_;
	EVP_PKEY* openssl_pkey_;	// Yes, we have both Crypto++ and OpenSSL-format private keys, because we have to use both libraries.

	X509* x509_;	// X509 structure pointer from OpenSSL

	CryptoPP::DL_PrivateKey_EC<CryptoPP::ECP>& gen_private_key();
	void write_key();

	void gen_certificate();
};

} /* namespace librevault */
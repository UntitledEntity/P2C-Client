#pragma once
#include <cryptopp/aes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/base64.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/hex.h>
#include <cryptopp/ccm.h>

#include <atlsecurity.h> 
#include <strsafe.h> 
#include <windows.h>
#include <string>
#include <stdio.h>
#include <iostream>

#include <curl/curl.h>

#include <shellapi.h>

#include <sstream> 
#include <iomanip> 

#include <fstream> 

#include <nlohmann/json.hpp>
#include "../xorstr.h"

#pragma comment(lib, "rpcrt4.lib")

namespace KeyAuth {

	class encryption {
	public:
		std::string name;
		static std::string encrypt_string(const std::string& plain_text, const std::string& key, const std::string& iv) {
			std::string cipher_text;

			try {
				CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryption;
				encryption.SetKeyWithIV((CryptoPP::byte*)key.c_str(), key.size(), (CryptoPP::byte*)iv.c_str());

				CryptoPP::StringSource encryptor(plain_text, true,
					new CryptoPP::StreamTransformationFilter(encryption,
						new CryptoPP::HexEncoder(
							new CryptoPP::StringSink(cipher_text),
							false
						)
					)
				);
			}
			catch (CryptoPP::Exception& ex) {
				exit(5000);
			}
			return cipher_text;
		}

		static std::string decrypt_string(const std::string& cipher_text, const std::string& key, const std::string& iv) {
			std::string plain_text;

			try {
				CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryption;
				decryption.SetKeyWithIV((CryptoPP::byte*)key.c_str(), key.size(), (CryptoPP::byte*)iv.c_str());

				CryptoPP::StringSource decryptor(cipher_text, true,
					new CryptoPP::HexDecoder(
						new CryptoPP::StreamTransformationFilter(decryption,
							new CryptoPP::StringSink(plain_text)
						)
					)
				);
			}
			catch (CryptoPP::Exception& ex) {
				exit(5000);
			}
			return plain_text;
		}

		static std::string sha256(const std::string& plain_text) {
			std::string hashed_text;
			CryptoPP::SHA256 hash;

			try {
				CryptoPP::StringSource hashing(plain_text, true,
					new CryptoPP::HashFilter(hash,
						new CryptoPP::HexEncoder(
							new CryptoPP::StringSink(hashed_text),
							false
						)
					)
				);
			}
			catch (CryptoPP::Exception& ex) {
				exit(5000);
			}

			return hashed_text;
		}

		static std::string encode(const std::string& plain_text) {
			std::string encoded_text;

			try {
				CryptoPP::StringSource encoding(plain_text, true,
					new CryptoPP::HexEncoder(
						new CryptoPP::StringSink(encoded_text),
						false
					)
				);
			}
			catch (CryptoPP::Exception& ex) {
				exit(5000);
			}

			return encoded_text;
		}

		static std::string decode(const std::string& encoded_text) {
			std::string out;

			try {
				CryptoPP::StringSource decoding(encoded_text, true,
					new CryptoPP::HexDecoder(
						new CryptoPP::StringSink(out)
					)
				);
			}
			catch (CryptoPP::Exception& ex) {
				exit(5000);
			}

			return out;
		}

		static std::string iv_key() {
			UUID uuid = { 0 };
			std::string guid;

			::UuidCreate(&uuid);

			RPC_CSTR szUuid = NULL;
			if (::UuidToStringA(&uuid, &szUuid) == RPC_S_OK)
			{
				guid = (char*)szUuid;
				::RpcStringFreeA(&szUuid);
			}

			return guid.substr(0, 16);
		}

		static std::string encrypt(std::string message, std::string enc_key, std::string iv) {
			enc_key = sha256(enc_key).substr(0, 32);
			iv = sha256(iv).substr(0, 16);
			return encrypt_string(message, enc_key, iv);
		}

		static std::string decrypt(std::string message, std::string enc_key, std::string iv) {
			enc_key = sha256(enc_key).substr(0, 32);

			iv = sha256(iv).substr(0, 16);

			return decrypt_string(message, enc_key, iv);
		}
	};

	class utils {
	public:

		static std::string get_hwid() {
			ATL::CAccessToken accessToken;
			ATL::CSid currentUserSid;
			if (accessToken.GetProcessToken(TOKEN_READ | TOKEN_QUERY) &&
				accessToken.GetUser(&currentUserSid))
				return std::string(CT2A(currentUserSid.Sid()));
		}

		static std::time_t string_to_timet(std::string timestamp) {
			auto cv = strtol(timestamp.c_str(), NULL, 10);

			return (time_t)cv;
		}

		static std::tm timet_to_tm(time_t timestamp) {
			std::tm context;

			localtime_s(&context, &timestamp);

			return context;
		}

		static ByteArray to_uc_vector(std::string value) {
			return std::vector<unsigned char>(value.data(), value.data() + value.length() + 1);
		}

	};


	inline auto iv = encryption::sha256(encryption::iv_key());
	class api {


	public:

		std::string name, ownerid, secret, version;

		api(std::string name, std::string ownerid, std::string secret, std::string version)
			: name(name), ownerid(ownerid), secret(secret), version(version) {}

		bool Init()
		{
			enckey = encryption::sha256(encryption::iv_key());

			auto data =
				STR("type=") + encryption::encode(STR("init")) +
				STR("&ver=") + encryption::encrypt(version, secret, iv) +
				STR("&hash=") + encryption::encode(STR("NULL")) +
				STR("&enckey=") + encryption::encrypt(enckey, secret, iv) +
				STR("&name=") + encryption::encode(name) +
				STR("&ownerid=") + encryption::encode(ownerid) +
				STR("&init_iv=") + iv;

			auto response = req(data);
			response = encryption::decrypt(response, secret, iv);
			auto json = response_decoder.parse(response);

			if (json[("success")])
			{
				sessionid = json[("sessionid")];
				load_app_data(json[("appinfo")]);
				Data.Initiated = true;

				return true;
			}
			else if (json[("message")] == "invalidver")
			{
				Data.InvalidVer = true;
				MessageBoxA(NULL, STR("New loader version detected. Click OK to download new version. "), STR("Intertwined Client"), MB_OK);

				std::string dl = json[("download")];
				ShellExecuteA(0, "open", dl.c_str(), 0, 0, SW_SHOWNORMAL);
				return false;
			}
			else {
				Data.ErrorMsg = json[("message")];
				return false;
			}
		}

		bool Login(std::string username, std::string password)
		{
			std::string hwid = utils::get_hwid();
			auto iv = encryption::sha256(encryption::iv_key());
			auto data =
				STR("type=") + encryption::encode(STR("login")) +
				STR("&username=") + encryption::encrypt(username, enckey, iv) +
				STR("&pass=") + encryption::encrypt(password, enckey, iv) +
				STR("&hwid=") + encryption::encrypt(hwid, enckey, iv) +
				STR("&sessionid=") + encryption::encode(sessionid) +
				STR("&name=") + encryption::encode(name) +
				STR("&ownerid=") + encryption::encode(ownerid) +
				STR("&init_iv=") + iv;

			auto response = req(data);
			response = encryption::decrypt(response, enckey, iv);

			auto json = response_decoder.parse(response);

			if (json[("success")])
			{
				load_user_data(json[("info")]);
				Data.LoggedIn = true;
				return true;
			}
			else if (json[("message")] == STR("The user is banned"))
			{
				Data.Banned = true;
				return false;
			}
			else
			{
				Data.ErrorMsg = json[("message")];
				return false;
			}
		}

		void SetVar(const std::string var, const std::string vardata) {

			auto iv = encryption::sha256(encryption::iv_key());
			auto data =
				STR("type=") + encryption::encode(STR("setvar")) +
				STR("&var=") + encryption::encrypt(var, enckey, iv) +
				STR("&data=") + encryption::encrypt(vardata, enckey, iv) +
				STR("&sessionid=") + encryption::encode(sessionid) +
				STR("&name=") + encryption::encode(name) +
				STR("&ownerid=") + encryption::encode(ownerid) +
				STR("&init_iv=") + iv;

			auto response = req(data);
			response = encryption::decrypt(response, enckey, iv);
			auto json = response_decoder.parse(response);

			if (!json[("success")])
				Data.ErrorMsg = json[("message")];
		}

		std::string GetVar(const std::string var) {

			auto iv = encryption::sha256(encryption::iv_key());
			auto data =
				STR("type=") + encryption::encode(STR("getvar")) +
				STR("&var=") + encryption::encrypt(var, enckey, iv) +
				STR("&sessionid=") + encryption::encode(sessionid) +
				STR("&name=") + encryption::encode(name) +
				STR("&ownerid=") + encryption::encode(ownerid) +
				STR("&init_iv=") + iv;
			auto response = req(data);
			response = encryption::decrypt(response, enckey, iv);
			auto json = response_decoder.parse(response);

			if (json[("success")])
			{
				return json[("response")];
			}
			else {
				Data.ErrorMsg = json[("message")];
				return STR("NULL");
			}
		}

		std::string Variable(const std::string var) {


			auto iv = encryption::sha256(encryption::iv_key());
			auto data =
				STR("type=") + encryption::encode(STR("var")) +
				STR("&varid=") + encryption::encrypt(var, enckey, iv) +
				STR("&sessionid=") + encryption::encode(sessionid) +
				STR("&name=") + encryption::encode(name) +
				STR("&ownerid=") + encryption::encode(ownerid) +
				STR("&init_iv=") + iv;

			auto response = req(data);
			response = encryption::decrypt(response, secret, iv);

			auto json = response_decoder.parse(response);
			return json[("success")] ? json[("message")] : "NULL";
		}

		bool CheckBlacklisted() {
			std::string hwid = utils::get_hwid();
			auto iv = encryption::sha256(encryption::iv_key());
			auto data =
				STR("type=") + encryption::encode(STR("checkblacklist")) +
				STR("&hwid=") + encryption::encrypt(hwid, enckey, iv) +
				STR("&sessionid=") + encryption::encode(sessionid) +
				STR("&name=") + encryption::encode(name) +
				STR("&ownerid=") + encryption::encode(ownerid) +
				STR("&init_iv=") + iv;

			auto response = req(data);
			response = encryption::decrypt(response, enckey, iv);

			auto json = response_decoder.parse(response);
			return json[("success")];
		}

		bool CheckExistingSession() {
			std::string hwid = utils::get_hwid();
			auto iv = encryption::sha256(encryption::iv_key());
			auto data =
				STR("type=") + encryption::encode(STR("check")) +
				STR("&sessionid=") + encryption::encode(sessionid) +
				STR("&name=") + encryption::encode(name) +
				STR("&ownerid=") + encryption::encode(ownerid) +
				STR("&init_iv=") + iv;

			auto response = req(data);
			response = encryption::decrypt(response, enckey, iv);

			auto json = response_decoder.parse(response);
			return json[("success")];
		}

		void BanUser() {

			if (!Data.LoggedIn)
				return;

			auto iv = encryption::sha256(encryption::iv_key());
			std::string hwid = utils::get_hwid();

			auto data =
				STR("type=") + encryption::encode(STR("ban")) +
				STR("&sessionid=") + encryption::encode(sessionid) +
				STR("&name=") + encryption::encode(name) +
				STR("&ownerid=") + encryption::encode(ownerid) +
				STR("&init_iv=") + iv;

			req(data);
		}

		void Log(std::string message) {

			auto iv = encryption::sha256(encryption::iv_key());

			char acUserName[100];
			DWORD nUserName = sizeof(acUserName);
			GetUserNameA(acUserName, &nUserName);
			std::string UsernamePC = acUserName;

			auto data =
				STR("type=") + encryption::encode(STR("log")) +
				STR("&pcuser=") + encryption::encrypt(UsernamePC, enckey, iv) +
				STR("&message=") + encryption::encrypt(message, enckey, iv) +
				STR("&sessionid=") + encryption::encode(sessionid) +
				STR("&name=") + encryption::encode(name) +
				STR("&ownerid=") + encryption::encode(ownerid) +
				STR("&init_iv=") + iv;

			req(data);
		}

		ByteArray Download(std::string fileid) {
			auto iv = encryption::sha256(encryption::iv_key());

			auto data =
				STR("type=") + encryption::encode(STR("file")) +
				STR("&fileid=") + encryption::encrypt(fileid, enckey, iv) +
				STR("&sessionid=") + encryption::encode(sessionid) +
				STR("&name=") + encryption::encode(name) +
				STR("&ownerid=") + encryption::encode(ownerid) +
				STR("&init_iv=") + iv;

			auto response = req(data);
			response = encryption::decrypt(response, enckey, iv);
			auto json = response_decoder.parse(response);

			if (!json["success"])
			{
				Data.ErrorMsg = json[("message")];
				return { };
			}

			auto file = encryption::decode(json["contents"]);

			return utils::to_uc_vector(file);
		}

		ByteArray RecieveData(bool beta)
		{
			auto iv = encryption::sha256(encryption::iv_key());
			auto to_uc_vector = [](std::string value) {
				return std::vector<unsigned char>(value.data(), value.data() + value.length() + 1);
			};

			std::string fileid = beta ? STR("BETAFILEID") : STR("FILEID");

			auto data =
				STR("type=") + encryption::encode(STR("file")) +
				STR("&fileid=") + encryption::encrypt(fileid, enckey, iv) +
				STR("&sessionid=") + encryption::encode(sessionid) +
				STR("&name=") + encryption::encode(name) +
				STR("&ownerid=") + encryption::encode(ownerid) +
				STR("&init_iv=") + iv;

			auto response = req(data);
			response = encryption::decrypt(response, enckey, iv);
			auto json = response_decoder.parse(response);

			if (!json["success"]) {
				Data.ErrorMsg = json[("message")];
				return to_uc_vector(nullptr);
			}

			return to_uc_vector(encryption::decode(json["contents"]));
		}

		ByteArray RecieveLegacyData(bool beta)
		{
			auto iv = encryption::sha256(encryption::iv_key());
			auto to_uc_vector = [](std::string value) {
				return std::vector<unsigned char>(value.data(), value.data() + value.length() + 1);
			};

			std::string fileid = beta ? STR("BETAFILEID") : STR("FILEID");

			auto data =
				STR("type=") + encryption::encode(STR("file")) +
				STR("&fileid=") + encryption::encrypt(fileid, enckey, iv) +
				STR("&sessionid=") + encryption::encode(sessionid) +
				STR("&name=") + encryption::encode(name) +
				STR("&ownerid=") + encryption::encode(ownerid) +
				STR("&init_iv=") + iv;

			auto response = req(data);
			response = encryption::decrypt(response, enckey, iv);
			auto json = response_decoder.parse(response);

			if (!json["success"]) {
				Data.ErrorMsg = json[("message")];
				return to_uc_vector(nullptr);
			}

			return to_uc_vector(encryption::decode(json["contents"]));
		}

		class user_data_class {
		public:
			std::string Name, Ip, Sub, HWID;
		};

		class app_data_class {
		public:
			std::string TotalUsers, UsersOnline, Version, TotalLicenses;
		};

		class data_class {
		public:
			std::string ErrorMsg;
			bool LoggedIn, Banned, InvalidVer, Initiated;
		};


		data_class Data;
		user_data_class User;
		app_data_class App;
	private:
		std::string sessionid, enckey;

		static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
			((std::string*)userp)->append((char*)contents, size * nmemb);
			return size * nmemb;
		}

		static std::string req(std::string data) {

			CURL* curl = curl_easy_init();

			if (!curl)
				return STR("NULL");

			std::string to_return;

			curl_easy_setopt(curl, CURLOPT_URL, STR("https://keyauth.win/api/1.0/"));

			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &to_return);

			auto code = curl_easy_perform(curl);

			if (code != CURLE_OK)
				MessageBoxA(0, curl_easy_strerror(code), 0, MB_ICONERROR);

			curl_easy_cleanup(curl);

			long http_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

			switch (http_code) {
			case 429:
				std::this_thread::sleep_for(std::chrono::seconds(1));
				return req(data);
			default:
				return STR("NULL");
			}

			return to_return;
		}

		class user_data_structure {
		public:
			std::string Name, Ip, Sub, HWID;
		};

		class app_data_structure {
		public:
			std::string TotalUsers, UsersOnline, Version, TotalLicenses;
		};

		class data_structure {
		public:
			std::string ErrorMsg;
			bool LoggedIn, Banned, InvalidVer;
		};

		void load_user_data(nlohmann::json data) {
			User.Name = data["username"];
			User.Ip = data["ip"];
			User.Sub = data["subscriptions"][0]["subscription"];
			User.HWID = data["hwid"];
		}

		void load_app_data(nlohmann::json data) {
			App.TotalUsers = data["numUsers"];
			App.UsersOnline = data["numOnlineUsers"];
			App.TotalLicenses = data["numKeys"];
			App.Version = data["version"];
		}

		nlohmann::json response_decoder;

	};
}

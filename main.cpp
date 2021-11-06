#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>

#include <evhttp.h>

#pragma comment(lib, "ws2_32.lib")

#include <GarrysMod/Lua/Interface.h>

void RemPosGyroscopeCallback(evhttp_request* request, [[maybe_unused]] void* unused);
void RemPosAccelerometerCallback(evhttp_request* request, [[maybe_unused]] void* unused);
void RemPosMagneticFieldCallback(evhttp_request* request, [[maybe_unused]] void* unused);

class RemPos {
public:
	struct RemPosValue {
		std::mutex mutex;
		bool updated = false;
		Vector value;
	};

public:
	void Intialize(GarrysMod::Lua::ILuaBase* LUA) {
		WSADATA WSAData;
		int result = WSAStartup(MAKEWORD(2, 2), &WSAData);
		if (result != 0) {
			std::string error = "WSAStartup error ";
			error += std::to_string(result);

			LUA->ThrowError(error.c_str());
			return;
		}

		event_base_ = event_base_new();

		if (!event_base_) {
			Destroy();
			LUA->ThrowError("Unable to initialize event_base");
		}

		event_http_ = evhttp_new(event_base_);

		if (!event_http_) {
			Destroy();
			LUA->ThrowError("Unable to initialize event_http");
		}

		evhttp_set_cb(event_http_, "/gyro", RemPosGyroscopeCallback, NULL);
		evhttp_set_cb(event_http_, "/accel", RemPosAccelerometerCallback, NULL);
		evhttp_set_cb(event_http_, "/magnet", RemPosMagneticFieldCallback, NULL);

		evhttp_set_gencb(event_http_, [](evhttp_request* request, void*) {
			evhttp_send_reply(request, HTTP_OK, "OK", NULL);
			}, NULL);

		evhttp_set_max_body_size(event_http_, 0U);

		const char* ip = LUA->Top() != 0 ? LUA->CheckString(1) : "0.0.0.0";
		const uint16_t port = LUA->Top() > 1 ? static_cast<uint16_t>(LUA->CheckNumber(2)) : 5555;

		auto handle = evhttp_bind_socket_with_handle(event_http_, ip, port);

		if (!handle) {
			std::string error = "Couldn't bind to ";

			error += ip;
			error += ":";
			error += std::to_string(port);

			Destroy();

			LUA->ThrowError(error.c_str());
			return;
		}

		event_loop_ = new std::thread([](event_base* base) {
			while (event_base_got_exit(base) == 0) {
				event_base_loop(base, EVLOOP_NONBLOCK);
			}
		}, event_base_);
		initialized_ = true;
	}

	void Destroy() {
		WSACleanup();

		if (event_http_) {
			event_base_loopexit(event_base_, NULL);

			if (event_loop_) {
				event_loop_->join();
				delete event_loop_;
			}

			evhttp_free(event_http_);
		}

		if (event_base_) {
			event_base_free(event_base_);
		}

		initialized_ = false;
	}

public:
	RemPosValue accelerometer_;
	RemPosValue gyroscope_;
	RemPosValue magnetic_field_;
	bool initialized_ = false;

private:
	event_base* event_base_ = nullptr;
	evhttp* event_http_ = nullptr;
	std::thread* event_loop_ = nullptr;
};

RemPos* rempos;

void RemPosGyroscopeCallback(evhttp_request* request, [[maybe_unused]] void* unused) {
	auto headers = evhttp_request_get_input_headers(request);
	evkeyval* header;

	for (header = headers->tqh_first; header; header = header->next.tqe_next) {
		if (strcmp(header->key, "gyro") == 0) {
			float x = 0, y = 0, z = 0;

			if (sscanf_s(header->value, "[%f, %f, %f]", &x, &y, &z) != 3) { return; }

			const std::lock_guard<std::mutex> lock(rempos->gyroscope_.mutex);

			rempos->gyroscope_.value = Vector(x, y, z);
			rempos->gyroscope_.updated = true;
			break;
		}
	}
	evhttp_send_reply(request, HTTP_OK, "OK", NULL);
}

void RemPosAccelerometerCallback(evhttp_request* request, [[maybe_unused]] void* unused) {
	auto headers = evhttp_request_get_input_headers(request);
	evkeyval* header;

	for (header = headers->tqh_first; header; header = header->next.tqe_next) {
		if (strcmp(header->key, "accel") == 0) {
			float x = 0, y = 0, z = 0;

			if (sscanf_s(header->value, "[%f, %f, %f]", &x, &y, &z) != 3) { return; }

			const std::lock_guard<std::mutex> lock(rempos->accelerometer_.mutex);

			rempos->accelerometer_.value = Vector(x, y, z);
			rempos->accelerometer_.updated = true;
			break;
		}
	}
	evhttp_send_reply(request, HTTP_OK, "OK", NULL);
}

void RemPosMagneticFieldCallback(evhttp_request* request, [[maybe_unused]] void* unused) {
	auto headers = evhttp_request_get_input_headers(request);
	evkeyval* header;

	for (header = headers->tqh_first; header; header = header->next.tqe_next) {
		if (strcmp(header->key, "magnet") == 0) {
			float x = 0, y = 0, z = 0;

			if (sscanf_s(header->value, "[%f, %f, %f]", &x, &y, &z) != 3) { return; }

			const std::lock_guard<std::mutex> lock(rempos->magnetic_field_.mutex);

			rempos->magnetic_field_.value = Vector(x, y, z);
			rempos->magnetic_field_.updated = true;
			break;
		}
	}
	evhttp_send_reply(request, HTTP_OK, "OK", NULL);
}


LUA_FUNCTION(RemPos_GetAccelerometer) {
	if (rempos) {
		const std::lock_guard<std::mutex> lock(rempos->accelerometer_.mutex);

		if (rempos->accelerometer_.updated) {
			LUA->PushVector(rempos->accelerometer_.value);
			rempos->accelerometer_.updated = false;
		}
		else {
			LUA->PushNil();
		}
		return 1;
	}
	else {
		LUA->PushNil();
		return 1;
	}
}

LUA_FUNCTION(RemPos_GetGyroscope) {
	if (rempos) {
		const std::lock_guard<std::mutex> lock(rempos->gyroscope_.mutex);

		if (rempos->gyroscope_.updated) {
			LUA->PushVector(rempos->gyroscope_.value);
			rempos->gyroscope_.updated = false;
		}
		else {
			LUA->PushNil();
		}
		return 1;
	}
	else {
		LUA->PushNil();
		return 1;
	}
}

LUA_FUNCTION(RemPos_GetMagneticField) {
	if (rempos) {
		const std::lock_guard<std::mutex> lock(rempos->magnetic_field_.mutex);

		if (rempos->magnetic_field_.updated) {
			LUA->PushVector(rempos->magnetic_field_.value);
			rempos->magnetic_field_.updated = false;
		}
		else {
			LUA->PushNil();
		}
		return 1;
	}
	else {
		LUA->PushNil();
		return 1;
	}
}

LUA_FUNCTION(RemPos_Initialize) {
	if (!rempos->initialized_) {
		rempos->Intialize(LUA);
	}
	return 0;
}

GMOD_MODULE_OPEN() {
	rempos = new RemPos();
	if (!rempos) {
		LUA->ThrowError("Couldn't allocate memory for RemPos object");
	}

	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);

	LUA->CreateTable();

	LUA->PushCFunction(RemPos_GetAccelerometer);
	LUA->SetField(-2, "GetAccelerometer");

	LUA->PushCFunction(RemPos_GetGyroscope);
	LUA->SetField(-2, "GetGyroscope");

	LUA->PushCFunction(RemPos_GetMagneticField);
	LUA->SetField(-2, "GetMagneticField");

	LUA->PushCFunction(RemPos_Initialize);
	LUA->SetField(-2, "Initialize");

	LUA->SetField(-2, "RemPos");

	return 0;
}

GMOD_MODULE_CLOSE() {
	if (rempos) {
		rempos->Destroy();
		delete rempos;
	}
	return 0;
}
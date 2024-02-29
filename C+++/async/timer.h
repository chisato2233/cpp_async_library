#pragma once
#include<chrono>

namespace cst {
	using game_time_point = std::chrono::duration<long double>;
	struct timer {

		inline static std::chrono::time_point<std::chrono::steady_clock> start_time_point;
		inline static game_time_point frame_time;

		static void start() {
			start_time_point = std::chrono::steady_clock::now();
		}

		static void update() {
			auto now = std::chrono::steady_clock::now();
			frame_time = now - last_frame_time_;
			last_frame_time_ = now;
		}

		static auto now()->game_time_point {
			using namespace std::chrono;
			return steady_clock::now() - start_time_point;
		}

	private:
		inline static std::chrono::time_point<std::chrono::steady_clock> last_frame_time_;

	};
}
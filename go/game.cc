/**
* Copyright (c) 2017-present, Facebook, Inc.
* All rights reserved.
*
* This source code is licensed under the BSD-style license found in the
* LICENSE file in the root directory of this source tree. An additional grant
* of patent rights can be found in the PATENTS file in the same directory.
*/

#include "game.h"
#include "go_game_specific.h"
#include "offpolicy_loader.h"
#include "online_player.h"

#include <fstream>

////////////////// GoGame /////////////////////
GoGame::GoGame(int game_idx, const GameOptions& options) : _options(options), _curr_loader_idx(0) {
    _game_idx = game_idx;
    if (options.seed == 0) {
        auto now = chrono::system_clock::now();
        auto now_ms = chrono::time_point_cast<chrono::milliseconds>(now);
        auto value = now_ms.time_since_epoch();
        long duration = value.count();
        _seed = (time(NULL) * 1000 + duration + _game_idx * 2341479) % 100000000;
        if (_options.verbose) std::cout << "[" << _game_idx << "] Seed:" << _seed << std::endl;
    } else {
        _seed = options.seed;
    }
    _rng.seed(_seed);
}

void GoGame::Init(AIComm *ai_comm) {
    assert(ai_comm);
    if (_options.online) {
        _loaders.emplace_back(new OnlinePlayer(ai_comm));
    } else {
        // Open many offline instances.
        for (int i = 0; i < _options.num_games_per_thread; ++i) {
            _loaders.emplace_back(new OfflineLoader(_options, _seed + _game_idx * i * 997 + i * 13773 + 7, ai_comm));
        }
    }
    if (_options.verbose) std::cout << "[" << _game_idx << "] Done with initialization" << std::endl;
}

void GoGame::Act(const std::atomic_bool& done) {
  // Randomly pick one loader
  _curr_loader_idx = _rng() % _loaders.size();
  Loader *loader = _loaders[_curr_loader_idx].get();

  loader->Act(done);
}

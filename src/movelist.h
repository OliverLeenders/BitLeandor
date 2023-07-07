#pragma once

#include "bit_move.h"

class movelist {
  public:
    movelist();
    struct ML_entry {
        bit_move m;
        uint16_t score;
    };

    ML_entry moves[265] = {};
    uint16_t size = 0;
    ~movelist();

  private:
};

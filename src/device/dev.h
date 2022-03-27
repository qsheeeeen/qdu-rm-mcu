#pragma once

/*
  SIGNAL是用bit位置区分的，一个SIGNAL只能包含一个高bit位。
  不同的SIGNAL不能有相同的高bit位。
*/
#define SIGNAL_DR16_RAW_REDY (1u << 7)
#define SIGNAL_IST8310_MAGN_NEW_DATA (1u << 8)
#define SIGNAL_IST8310_MAGN_RAW_REDY (1u << 9)

#define SIGNAL_REFEREE_RAW_REDY (1u << 10)
#define SIGNAL_REFEREE_PACKET_SENT (1u << 11)
#define SIGNAL_REFEREE_FAST_REFRESH_UI (1u << 12)
#define SIGNAL_REFEREE_SLOW_REFRESH_UI (1u << 13)

/* -*- mode: c++ -*-
 * Copyright (C) 2025  Keyboard.io, Inc.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <Kaleidoscope.h>
#include <Kaleidoscope-PloverHID.h>

// *INDENT-OFF*
KEYMAPS(
    [0] = KEYMAP_STACKED
    (
        // Left hand - basic steno keys
        PH(S_L), PH(T_L), PH(K_L), PH(P_L), PH(W_L), PH(H_L), PH(R_L),
        PH(S_L), PH(T_L), PH(K_L), PH(P_L), PH(W_L), PH(H_L), PH(R_L),
        PH(S_L), PH(T_L), PH(K_L), PH(P_L), PH(W_L), PH(H_L),
        PH(NUM), PH(S_L), PH(T_L), PH(K_L), PH(P_L), PH(W_L), PH(H_L),
        PH(A), PH(O), ___, ___,
        PH(STAR),

        // Right hand - basic steno keys  
        PH(F_R), PH(R_R), PH(P_R), PH(B_R), PH(L_R), PH(G_R), PH(T_R),
        PH(F_R), PH(R_R), PH(P_R), PH(B_R), PH(L_R), PH(G_R), PH(T_R),
        PH(F_R), PH(R_R), PH(P_R), PH(B_R), PH(L_R), PH(G_R),
        PH(F_R), PH(R_R), PH(P_R), PH(B_R), PH(L_R), PH(G_R), PH(T_R),
        ___, ___, PH(E), PH(U),
        PH(S_R)
    ),
    [1] = KEYMAP_STACKED
    (
        // Extended keys test
        PH(X1), PH(X2), PH(X3), PH(X4), PH(X5), PH(X6), PH(X7),
        PH(X8), PH(X9), PH(X10), PH(X11), PH(X12), PH(X13), PH(X14),
        PH(X15), PH(X16), PH(X17), PH(X18), PH(X19), PH(X20),
        PH(X21), PH(X22), PH(X23), PH(X24), PH(X25), PH(X26), PH(X27),
        PH(X28), PH(X29), ___, ___,
        PH(X30),

        PH(X31), PH(X32), PH(X33), PH(X34), PH(X35), PH(X36), PH(X37),
        PH(X38), PH(X39), ___, ___, ___, ___, ___,
        ___, ___, ___, ___, ___, ___,
        ___, ___, ___, ___, ___, ___, ___,
        ___, ___, ___, ___,
        ___
    )
)
// *INDENT-ON*

KALEIDOSCOPE_INIT_PLUGINS(PloverHID);

void setup() {
  Kaleidoscope.setup();
}

void loop() {
  Kaleidoscope.loop();
}

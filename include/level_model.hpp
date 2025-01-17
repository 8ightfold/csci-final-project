#ifndef PROJECT3_TEST_LEVEL_MODEL_HPP
#define PROJECT3_TEST_LEVEL_MODEL_HPP

#define LEVEL_VERTEX_COUNT 84
const S3L_Unit levelVertices[LEVEL_VERTEX_COUNT * 3] = {
        -1990,  1285, -2716,        // 0
        -1422,  1285, -2951,        // 3
        5395,     4, -2716,        // 6
        -5409,  4626,  2716,        // 9
        -5409,     4,  2716,        // 12
        -5409,  4626, -2716,        // 15
        -5409,     0,  -598,        // 18
        -1187,     4,  2716,        // 21
        -1187,  1285, -3519,        // 24
        964,  1285, -2716,        // 27
        -5409,  2326,  2716,        // 30
        964,  1285, -6473,        // 33
        -5409,  4626,     0,        // 36
        5395,     4,   513,        // 39
        964,  1285,  -598,        // 42
        -3302,  1285,  -598,        // 45
        -3302,     4,  -598,        // 48
        3179,  2102, -6473,        // 51
        5395,  4626, -6473,        // 54
        -1187,  4626, -6473,        // 57
        -1187,  1285, -6473,        // 60
        964,     4, -2716,        // 63
        -5409,  1285, -2716,        // 66
        3194,     4,  2716,        // 69
        -5409,  1285,  -598,        // 72
        964,     4,  -598,        // 75
        2103,  4626, -6473,        // 78
        -1187,  4626, -3519,        // 81
        -1990,  4626, -2716,        // 84
        -1422,  4626, -2951,        // 87
        -3302,     4,  1353,        // 90
        -5409,     4,  1353,        // 93
        5395,     4, -4594,        // 96
        964,     4, -4594,        // 99
        3179,     4, -4594,        // 102
        5395,  2102, -6473,        // 105
        5395,  2102, -4594,        // 108
        3179,  2102, -4594,        // 111
        964,  1285, -4594,        // 114
        196,     8,   681,        // 117
        196,  4626,   681,        // 120
        558,     8,   531,        // 123
        558,  4626,   531,        // 126
        708,     8,   169,        // 129
        708,  4626,   169,        // 132
        558,     8,  -192,        // 135
        558,  4626,  -192,        // 138
        196,     8,  -342,        // 141
        196,  4626,  -342,        // 144
        -165,     8,  -192,        // 147
        -165,  4626,  -192,        // 150
        -315,     8,   169,        // 153
        -315,  4626,   169,        // 156
        -165,     8,   531,        // 159
        -165,  4626,   531,        // 162
        5395,  4626,   513,        // 165
        3194,  4626,  2716,        // 168
        2216,   869, -2759,        // 171
        2216,  1002, -3306,        // 174
        2603,   869, -2919,        // 177
        2887,   507, -2636,        // 180
        2990,    13, -2532,        // 183
        2764,   869, -3306,        // 186
        3164,   507, -3306,        // 189
        3311,    13, -3306,        // 192
        2603,   869, -3693,        // 195
        2887,   507, -3976,        // 198
        2990,    13, -4080,        // 201
        2216,   869, -3853,        // 204
        2216,   507, -4254,        // 207
        2216,    13, -4400,        // 210
        1830,   869, -3693,        // 213
        1546,   507, -3976,        // 216
        1443,    13, -4080,        // 219
        1669,   869, -3306,        // 222
        1269,   507, -3306,        // 225
        1122,    13, -3306,        // 228
        1830,   869, -2919,        // 231
        1546,   507, -2636,        // 234
        1443,    13, -2532,        // 237
        2216,   507, -2358,        // 240
        2216,    13, -2212,        // 243
        -1187,  4626,  2716,        // 246
        5395,  4626, -2716         // 249
}; // levelVertices

#define LEVEL_TRIANGLE_COUNT 146
const S3L_Index levelTriangleIndices[LEVEL_TRIANGLE_COUNT * 3] = {
        25,     9,    21,        // 0
        10,    12,    24,        // 3
        1,    14,     0,        // 6
        16,    14,    25,        // 9
        8,     9,     1,        // 12
        24,    30,    31,        // 15
        22,    15,    24,        // 18
        13,    23,    25,        // 21
        15,    16,    30,        // 24
        24,     5,    22,        // 27
        27,    20,     8,        // 30
        11,    38,    20,        // 33
        14,     1,     9,        // 36
        37,    35,    36,        // 39
        14,    15,     0,        // 42
        29,     0,    28,        // 45
        27,     1,    29,        // 48
        22,    28,     0,        // 51
        4,    30,     7,        // 54
        24,    31,    10,        // 57
        10,    31,     4,        // 60
        17,    38,    11,        // 63
        20,    38,     8,        // 66
        9,     8,    38,        // 69
        38,    34,    33,        // 72
        9,    38,    21,        // 75
        38,    33,    21,        // 78
        38,    37,    34,        // 81
        36,    34,    37,        // 84
        16,    47,    49,        // 87
        18,    17,    26,        // 90
        11,    26,    17,        // 93
        7,    30,    23,        // 96
        47,    25,    45,        // 99
        40,    41,    39,        // 102
        41,    44,    43,        // 105
        44,    45,    43,        // 108
        46,    47,    45,        // 111
        48,    49,    47,        // 114
        50,    51,    49,        // 117
        52,    53,    51,        // 120
        54,    39,    53,        // 123
        25,    43,    45,        // 126
        43,    23,    41,        // 129
        41,    23,    39,        // 132
        23,    30,    39,        // 135
        39,    30,    53,        // 138
        53,    30,    51,        // 141
        49,    51,    16,        // 144
        30,    16,    51,        // 147
        13,    56,    23,        // 150
        80,    61,    81,        // 153
        57,    58,    59,        // 156
        80,    59,    60,        // 159
        60,    64,    61,        // 162
        59,    58,    62,        // 165
        60,    62,    63,        // 168
        63,    67,    64,        // 171
        62,    58,    65,        // 174
        62,    66,    63,        // 177
        65,    58,    68,        // 180
        65,    69,    66,        // 183
        67,    69,    70,        // 186
        68,    72,    69,        // 189
        69,    73,    70,        // 192
        68,    58,    71,        // 195
        71,    75,    72,        // 198
        72,    76,    73,        // 201
        71,    58,    74,        // 204
        74,    78,    75,        // 207
        76,    78,    79,        // 210
        74,    58,    77,        // 213
        77,    80,    78,        // 216
        79,    80,    81,        // 219
        77,    58,    57,        // 222
        70,    73,    67,        // 225
        33,    34,    70,        // 228
        73,    33,    70,        // 231
        76,    33,    73,        // 234
        33,    76,    21,        // 237
        76,    79,    21,        // 240
        21,    79,    25,        // 243
        79,    81,    25,        // 246
        25,    81,    13,        // 249
        81,    61,    13,        // 252
        13,    61,     2,        // 255
        61,    64,     2,        // 258
        2,    64,    32,        // 261
        64,    67,    32,        // 264
        34,    32,    67,        // 267
        67,    70,    34,        // 270
        82,    10,     4,        // 273
        4,     7,    82,        // 276
        10,    82,     3,        // 279
        7,    23,    82,        // 282
        23,    56,    82,        // 285
        13,     2,    83,        // 288
        55,    13,    83,        // 291
        36,    83,     2,        // 294
        32,    36,     2,        // 297
        36,    18,    83,        // 300
        35,    18,    36,        // 303
        25,    14,     9,        // 306
        10,     3,    12,        // 309
        16,    15,    14,        // 312
        24,    15,    30,        // 315
        22,     0,    15,        // 318
        24,    12,     5,        // 321
        27,    19,    20,        // 324
        37,    17,    35,        // 327
        29,     1,     0,        // 330
        27,     8,     1,        // 333
        22,     5,    28,        // 336
        4,    31,    30,        // 339
        17,    37,    38,        // 342
        36,    32,    34,        // 345
        16,    25,    47,        // 348
        18,    35,    17,        // 351
        40,    42,    41,        // 354
        41,    42,    44,        // 357
        44,    46,    45,        // 360
        46,    48,    47,        // 363
        48,    50,    49,        // 366
        50,    52,    51,        // 369
        52,    54,    53,        // 372
        54,    40,    39,        // 375
        25,    23,    43,        // 378
        13,    55,    56,        // 381
        80,    60,    61,        // 384
        80,    57,    59,        // 387
        60,    63,    64,        // 390
        60,    59,    62,        // 393
        63,    66,    67,        // 396
        62,    65,    66,        // 399
        65,    68,    69,        // 402
        67,    66,    69,        // 405
        68,    71,    72,        // 408
        69,    72,    73,        // 411
        71,    74,    75,        // 414
        72,    75,    76,        // 417
        74,    77,    78,        // 420
        76,    75,    78,        // 423
        77,    57,    80,        // 426
        79,    78,    80,        // 429
        19,    11,    20,        // 432
        19,    26,    11         // 435
}; // levelTriangleIndices

S3L_Model3D levelModel;

void levelModelInit(void)
{
    S3L_model3DInit(
            levelVertices,
            LEVEL_VERTEX_COUNT,
            levelTriangleIndices,
            LEVEL_TRIANGLE_COUNT,
            &levelModel);
}

#endif //PROJECT3_TEST_LEVEL_MODEL_HPP

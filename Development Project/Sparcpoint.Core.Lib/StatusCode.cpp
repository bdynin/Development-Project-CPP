#ifndef ITI_LIB_HTTP_STATUSCODE_CPP
#define ITI_LIB_HTTP_STATUSCODE_CPP

#include "pch.h"

#include "StatusCode.h"

#include <array>
#include <fmt/format.h>
#include <stdexcept>

#include "StrUtils.h"

using iti::http::StatusCode;
using iti::strutils::empty_sv;

// Helpers
// ----------------------------------------------------------------------------

static const constexpr std::array<bool, 512> isStatusCodeTable = {
    false, // 0
    false, // 1
    false, // 2
    false, // 3
    false, // 4
    false, // 5
    false, // 6
    false, // 7
    false, // 8
    false, // 9
    false, // 10
    false, // 11
    false, // 12
    false, // 13
    false, // 14
    false, // 15
    false, // 16
    false, // 17
    false, // 18
    false, // 19
    false, // 20
    false, // 21
    false, // 22
    false, // 23
    false, // 24
    false, // 25
    false, // 26
    false, // 27
    false, // 28
    false, // 29
    false, // 30
    false, // 31
    false, // 32
    false, // 33
    false, // 34
    false, // 35
    false, // 36
    false, // 37
    false, // 38
    false, // 39
    false, // 40
    false, // 41
    false, // 42
    false, // 43
    false, // 44
    false, // 45
    false, // 46
    false, // 47
    false, // 48
    false, // 49
    false, // 50
    false, // 51
    false, // 52
    false, // 53
    false, // 54
    false, // 55
    false, // 56
    false, // 57
    false, // 58
    false, // 59
    false, // 60
    false, // 61
    false, // 62
    false, // 63
    false, // 64
    false, // 65
    false, // 66
    false, // 67
    false, // 68
    false, // 69
    false, // 70
    false, // 71
    false, // 72
    false, // 73
    false, // 74
    false, // 75
    false, // 76
    false, // 77
    false, // 78
    false, // 79
    false, // 80
    false, // 81
    false, // 82
    false, // 83
    false, // 84
    false, // 85
    false, // 86
    false, // 87
    false, // 88
    false, // 89
    false, // 90
    false, // 91
    false, // 92
    false, // 93
    false, // 94
    false, // 95
    false, // 96
    false, // 97
    false, // 98
    false, // 99
    true,  // 100 Continue
    true,  // 101 SwitchingProtocol
    true,  // 102 Processing
    true,  // 103 EarlyHints
    false, // 104
    false, // 105
    false, // 106
    false, // 107
    false, // 108
    false, // 109
    false, // 110
    false, // 111
    false, // 112
    false, // 113
    false, // 114
    false, // 115
    false, // 116
    false, // 117
    false, // 118
    false, // 119
    false, // 120
    false, // 121
    false, // 122
    false, // 123
    false, // 124
    false, // 125
    false, // 126
    false, // 127
    false, // 128
    false, // 129
    false, // 130
    false, // 131
    false, // 132
    false, // 133
    false, // 134
    false, // 135
    false, // 136
    false, // 137
    false, // 138
    false, // 139
    false, // 140
    false, // 141
    false, // 142
    false, // 143
    false, // 144
    false, // 145
    false, // 146
    false, // 147
    false, // 148
    false, // 149
    false, // 150
    false, // 151
    false, // 152
    false, // 153
    false, // 154
    false, // 155
    false, // 156
    false, // 157
    false, // 158
    false, // 159
    false, // 160
    false, // 161
    false, // 162
    false, // 163
    false, // 164
    false, // 165
    false, // 166
    false, // 167
    false, // 168
    false, // 169
    false, // 170
    false, // 171
    false, // 172
    false, // 173
    false, // 174
    false, // 175
    false, // 176
    false, // 177
    false, // 178
    false, // 179
    false, // 180
    false, // 181
    false, // 182
    false, // 183
    false, // 184
    false, // 185
    false, // 186
    false, // 187
    false, // 188
    false, // 189
    false, // 190
    false, // 191
    false, // 192
    false, // 193
    false, // 194
    false, // 195
    false, // 196
    false, // 197
    false, // 198
    false, // 199
    true,  // 200 OK
    true,  // 201 Created
    true,  // 202 Accepted
    true,  // 203 NonAuthoritativeInfo
    true,  // 204 NoContent
    true,  // 205 ResetContent
    true,  // 206 ParitalContent
    true,  // 207 MultiStatus
    true,  // 208 AlreadyReported
    false, // 209
    false, // 210
    false, // 211
    false, // 212
    false, // 213
    false, // 214
    false, // 215
    false, // 216
    false, // 217
    false, // 218
    false, // 219
    false, // 220
    false, // 221
    false, // 222
    false, // 223
    false, // 224
    false, // 225
    true,  // 226 IMUsed
    false, // 227
    false, // 228
    false, // 229
    false, // 230
    false, // 231
    false, // 232
    false, // 233
    false, // 234
    false, // 235
    false, // 236
    false, // 237
    false, // 238
    false, // 239
    false, // 240
    false, // 241
    false, // 242
    false, // 243
    false, // 244
    false, // 245
    false, // 246
    false, // 247
    false, // 248
    false, // 249
    false, // 250
    false, // 251
    false, // 252
    false, // 253
    false, // 254
    false, // 255
    false, // 256
    false, // 257
    false, // 258
    false, // 259
    false, // 260
    false, // 261
    false, // 262
    false, // 263
    false, // 264
    false, // 265
    false, // 266
    false, // 267
    false, // 268
    false, // 269
    false, // 270
    false, // 271
    false, // 272
    false, // 273
    false, // 274
    false, // 275
    false, // 276
    false, // 277
    false, // 278
    false, // 279
    false, // 280
    false, // 281
    false, // 282
    false, // 283
    false, // 284
    false, // 285
    false, // 286
    false, // 287
    false, // 288
    false, // 289
    false, // 290
    false, // 291
    false, // 292
    false, // 293
    false, // 294
    false, // 295
    false, // 296
    false, // 297
    false, // 298
    false, // 299
    true,  // 300 MultipleChoice
    true,  // 301 MovedPermanently
    true,  // 302 Found
    true,  // 303 SeeOther
    true,  // 304 NotModified
    true,  // 305 UseProxy
    false, // 306
    true,  // 307 TemporaryRedirect
    true,  // 308 PermanentRedirect
    false, // 309
    false, // 310
    false, // 311
    false, // 312
    false, // 313
    false, // 314
    false, // 315
    false, // 316
    false, // 317
    false, // 318
    false, // 319
    false, // 320
    false, // 321
    false, // 322
    false, // 323
    false, // 324
    false, // 325
    false, // 326
    false, // 327
    false, // 328
    false, // 329
    false, // 330
    false, // 331
    false, // 332
    false, // 333
    false, // 334
    false, // 335
    false, // 336
    false, // 337
    false, // 338
    false, // 339
    false, // 340
    false, // 341
    false, // 342
    false, // 343
    false, // 344
    false, // 345
    false, // 346
    false, // 347
    false, // 348
    false, // 349
    false, // 350
    false, // 351
    false, // 352
    false, // 353
    false, // 354
    false, // 355
    false, // 356
    false, // 357
    false, // 358
    false, // 359
    false, // 360
    false, // 361
    false, // 362
    false, // 363
    false, // 364
    false, // 365
    false, // 366
    false, // 367
    false, // 368
    false, // 369
    false, // 370
    false, // 371
    false, // 372
    false, // 373
    false, // 374
    false, // 375
    false, // 376
    false, // 377
    false, // 378
    false, // 379
    false, // 380
    false, // 381
    false, // 382
    false, // 383
    false, // 384
    false, // 385
    false, // 386
    false, // 387
    false, // 388
    false, // 389
    false, // 390
    false, // 391
    false, // 392
    false, // 393
    false, // 394
    false, // 395
    false, // 396
    false, // 397
    false, // 398
    false, // 399
    true,  // 400 BadRequest
    true,  // 401 Unauthorized
    true,  // 402 PaymentRequired
    true,  // 403 Forbidden
    true,  // 404 NotFound
    true,  // 405 MethodNotAllowed
    true,  // 406 NotAcceptable
    true,  // 407 ProxyAuthRequired
    true,  // 408 RequestTimeout
    true,  // 409 Conflict
    true,  // 410 Gone
    true,  // 411 LengthRequired
    true,  // 412 PreconditionFailed
    true,  // 413 PayloadTooLarge
    true,  // 414 UriTooLong
    true,  // 415 UnsupportedMediaType
    true,  // 416 RangeNotSatisfiable
    true,  // 417 ExpectationFailed
    true,  // 418 Teapot
    false, // 419
    false, // 420
    true,  // 421 MisdirectedRequest
    true,  // 422 UnprocessableEntity
    true,  // 423 Locked
    true,  // 424 FailedDependency
    true,  // 425 TooEarly
    true,  // 426 UpgradeRequired
    false, // 427
    true,  // 428 PreconditionRequired
    true,  // 429 TooManyRequests
    false, // 430
    true,  // 431 RequestHeaderFieldsTooLarge
    false, // 432
    false, // 433
    false, // 434
    false, // 435
    false, // 436
    false, // 437
    false, // 438
    false, // 439
    false, // 440
    false, // 441
    false, // 442
    false, // 443
    false, // 444
    false, // 445
    false, // 446
    false, // 447
    false, // 448
    false, // 449
    false, // 450
    true,  // 451 UnavailableForLegalReasons
    false, // 452
    false, // 453
    false, // 454
    false, // 455
    false, // 456
    false, // 457
    false, // 458
    false, // 459
    false, // 460
    false, // 461
    false, // 462
    false, // 463
    false, // 464
    false, // 465
    false, // 466
    false, // 467
    false, // 468
    false, // 469
    false, // 470
    false, // 471
    false, // 472
    false, // 473
    false, // 474
    false, // 475
    false, // 476
    false, // 477
    false, // 478
    false, // 479
    false, // 480
    false, // 481
    false, // 482
    false, // 483
    false, // 484
    false, // 485
    false, // 486
    false, // 487
    false, // 488
    false, // 489
    false, // 490
    false, // 491
    false, // 492
    false, // 493
    false, // 494
    false, // 495
    false, // 496
    false, // 497
    false, // 498
    false, // 499
    true,  // 500 InternalServerError
    true,  // 501 NotImplemented
    true,  // 502 BadGateway
    true,  // 503 ServiceUnavailable
    true,  // 504 GatewayTimeout
    true,  // 505 HttpVersionNotSupported
    true,  // 506 VariantAlsoNegotiates
    true,  // 507 InsufficientStorage
    true,  // 508 LoopDetected
    false, // 509
    true,  // 510 NotExtended
    true,  // 511 NetworkAuthRequired
};

static const constexpr std::array<std::string_view, 512> statusCodeStrTable = {
    empty_sv,                              // 0
    empty_sv,                              // 1
    empty_sv,                              // 2
    empty_sv,                              // 3
    empty_sv,                              // 4
    empty_sv,                              // 5
    empty_sv,                              // 6
    empty_sv,                              // 7
    empty_sv,                              // 8
    empty_sv,                              // 9
    empty_sv,                              // 10
    empty_sv,                              // 11
    empty_sv,                              // 12
    empty_sv,                              // 13
    empty_sv,                              // 14
    empty_sv,                              // 15
    empty_sv,                              // 16
    empty_sv,                              // 17
    empty_sv,                              // 18
    empty_sv,                              // 19
    empty_sv,                              // 20
    empty_sv,                              // 21
    empty_sv,                              // 22
    empty_sv,                              // 23
    empty_sv,                              // 24
    empty_sv,                              // 25
    empty_sv,                              // 26
    empty_sv,                              // 27
    empty_sv,                              // 28
    empty_sv,                              // 29
    empty_sv,                              // 30
    empty_sv,                              // 31
    empty_sv,                              // 32
    empty_sv,                              // 33
    empty_sv,                              // 34
    empty_sv,                              // 35
    empty_sv,                              // 36
    empty_sv,                              // 37
    empty_sv,                              // 38
    empty_sv,                              // 39
    empty_sv,                              // 40
    empty_sv,                              // 41
    empty_sv,                              // 42
    empty_sv,                              // 43
    empty_sv,                              // 44
    empty_sv,                              // 45
    empty_sv,                              // 46
    empty_sv,                              // 47
    empty_sv,                              // 48
    empty_sv,                              // 49
    empty_sv,                              // 50
    empty_sv,                              // 51
    empty_sv,                              // 52
    empty_sv,                              // 53
    empty_sv,                              // 54
    empty_sv,                              // 55
    empty_sv,                              // 56
    empty_sv,                              // 57
    empty_sv,                              // 58
    empty_sv,                              // 59
    empty_sv,                              // 60
    empty_sv,                              // 61
    empty_sv,                              // 62
    empty_sv,                              // 63
    empty_sv,                              // 64
    empty_sv,                              // 65
    empty_sv,                              // 66
    empty_sv,                              // 67
    empty_sv,                              // 68
    empty_sv,                              // 69
    empty_sv,                              // 70
    empty_sv,                              // 71
    empty_sv,                              // 72
    empty_sv,                              // 73
    empty_sv,                              // 74
    empty_sv,                              // 75
    empty_sv,                              // 76
    empty_sv,                              // 77
    empty_sv,                              // 78
    empty_sv,                              // 79
    empty_sv,                              // 80
    empty_sv,                              // 81
    empty_sv,                              // 82
    empty_sv,                              // 83
    empty_sv,                              // 84
    empty_sv,                              // 85
    empty_sv,                              // 86
    empty_sv,                              // 87
    empty_sv,                              // 88
    empty_sv,                              // 89
    empty_sv,                              // 90
    empty_sv,                              // 91
    empty_sv,                              // 92
    empty_sv,                              // 93
    empty_sv,                              // 94
    empty_sv,                              // 95
    empty_sv,                              // 96
    empty_sv,                              // 97
    empty_sv,                              // 98
    empty_sv,                              // 99
    "100 Continue",                        // 100
    "101 Switching Protocol",              // 101
    "102 Processing",                      // 102
    "103 Early Hints",                     // 103
    empty_sv,                              // 104
    empty_sv,                              // 105
    empty_sv,                              // 106
    empty_sv,                              // 107
    empty_sv,                              // 108
    empty_sv,                              // 109
    empty_sv,                              // 110
    empty_sv,                              // 111
    empty_sv,                              // 112
    empty_sv,                              // 113
    empty_sv,                              // 114
    empty_sv,                              // 115
    empty_sv,                              // 116
    empty_sv,                              // 117
    empty_sv,                              // 118
    empty_sv,                              // 119
    empty_sv,                              // 120
    empty_sv,                              // 121
    empty_sv,                              // 122
    empty_sv,                              // 123
    empty_sv,                              // 124
    empty_sv,                              // 125
    empty_sv,                              // 126
    empty_sv,                              // 127
    empty_sv,                              // 128
    empty_sv,                              // 129
    empty_sv,                              // 130
    empty_sv,                              // 131
    empty_sv,                              // 132
    empty_sv,                              // 133
    empty_sv,                              // 134
    empty_sv,                              // 135
    empty_sv,                              // 136
    empty_sv,                              // 137
    empty_sv,                              // 138
    empty_sv,                              // 139
    empty_sv,                              // 140
    empty_sv,                              // 141
    empty_sv,                              // 142
    empty_sv,                              // 143
    empty_sv,                              // 144
    empty_sv,                              // 145
    empty_sv,                              // 146
    empty_sv,                              // 147
    empty_sv,                              // 148
    empty_sv,                              // 149
    empty_sv,                              // 150
    empty_sv,                              // 151
    empty_sv,                              // 152
    empty_sv,                              // 153
    empty_sv,                              // 154
    empty_sv,                              // 155
    empty_sv,                              // 156
    empty_sv,                              // 157
    empty_sv,                              // 158
    empty_sv,                              // 159
    empty_sv,                              // 160
    empty_sv,                              // 161
    empty_sv,                              // 162
    empty_sv,                              // 163
    empty_sv,                              // 164
    empty_sv,                              // 165
    empty_sv,                              // 166
    empty_sv,                              // 167
    empty_sv,                              // 168
    empty_sv,                              // 169
    empty_sv,                              // 170
    empty_sv,                              // 171
    empty_sv,                              // 172
    empty_sv,                              // 173
    empty_sv,                              // 174
    empty_sv,                              // 175
    empty_sv,                              // 176
    empty_sv,                              // 177
    empty_sv,                              // 178
    empty_sv,                              // 179
    empty_sv,                              // 180
    empty_sv,                              // 181
    empty_sv,                              // 182
    empty_sv,                              // 183
    empty_sv,                              // 184
    empty_sv,                              // 185
    empty_sv,                              // 186
    empty_sv,                              // 187
    empty_sv,                              // 188
    empty_sv,                              // 189
    empty_sv,                              // 190
    empty_sv,                              // 191
    empty_sv,                              // 192
    empty_sv,                              // 193
    empty_sv,                              // 194
    empty_sv,                              // 195
    empty_sv,                              // 196
    empty_sv,                              // 197
    empty_sv,                              // 198
    empty_sv,                              // 199
    "200 OK",                              // 200
    "201 Created",                         // 201
    "202 Accepted",                        // 202
    "203 Non-Authoritative Information",   // 203
    "204 No Content",                      // 204
    "205 Reset Content",                   // 205
    "206 Parital Content",                 // 206
    "207 Multi-Status",                    // 207
    "208 Already Reported",                // 208
    empty_sv,                              // 209
    empty_sv,                              // 210
    empty_sv,                              // 211
    empty_sv,                              // 212
    empty_sv,                              // 213
    empty_sv,                              // 214
    empty_sv,                              // 215
    empty_sv,                              // 216
    empty_sv,                              // 217
    empty_sv,                              // 218
    empty_sv,                              // 219
    empty_sv,                              // 220
    empty_sv,                              // 221
    empty_sv,                              // 222
    empty_sv,                              // 223
    empty_sv,                              // 224
    empty_sv,                              // 225
    "226 IM Used",                         // 226
    empty_sv,                              // 227
    empty_sv,                              // 228
    empty_sv,                              // 229
    empty_sv,                              // 230
    empty_sv,                              // 231
    empty_sv,                              // 232
    empty_sv,                              // 233
    empty_sv,                              // 234
    empty_sv,                              // 235
    empty_sv,                              // 236
    empty_sv,                              // 237
    empty_sv,                              // 238
    empty_sv,                              // 239
    empty_sv,                              // 240
    empty_sv,                              // 241
    empty_sv,                              // 242
    empty_sv,                              // 243
    empty_sv,                              // 244
    empty_sv,                              // 245
    empty_sv,                              // 246
    empty_sv,                              // 247
    empty_sv,                              // 248
    empty_sv,                              // 249
    empty_sv,                              // 250
    empty_sv,                              // 251
    empty_sv,                              // 252
    empty_sv,                              // 253
    empty_sv,                              // 254
    empty_sv,                              // 255
    empty_sv,                              // 256
    empty_sv,                              // 257
    empty_sv,                              // 258
    empty_sv,                              // 259
    empty_sv,                              // 260
    empty_sv,                              // 261
    empty_sv,                              // 262
    empty_sv,                              // 263
    empty_sv,                              // 264
    empty_sv,                              // 265
    empty_sv,                              // 266
    empty_sv,                              // 267
    empty_sv,                              // 268
    empty_sv,                              // 269
    empty_sv,                              // 270
    empty_sv,                              // 271
    empty_sv,                              // 272
    empty_sv,                              // 273
    empty_sv,                              // 274
    empty_sv,                              // 275
    empty_sv,                              // 276
    empty_sv,                              // 277
    empty_sv,                              // 278
    empty_sv,                              // 279
    empty_sv,                              // 280
    empty_sv,                              // 281
    empty_sv,                              // 282
    empty_sv,                              // 283
    empty_sv,                              // 284
    empty_sv,                              // 285
    empty_sv,                              // 286
    empty_sv,                              // 287
    empty_sv,                              // 288
    empty_sv,                              // 289
    empty_sv,                              // 290
    empty_sv,                              // 291
    empty_sv,                              // 292
    empty_sv,                              // 293
    empty_sv,                              // 294
    empty_sv,                              // 295
    empty_sv,                              // 296
    empty_sv,                              // 297
    empty_sv,                              // 298
    empty_sv,                              // 299
    "300 Multiple Choice",                 // 300
    "301 Moved Permanently",               // 301
    "302 Found",                           // 302
    "303 SeeOther",                        // 303
    "304 Not Modified",                    // 304
    "305 Use Proxy",                       // 305
    empty_sv,                              // 306
    "307 Temporary Redirect",              // 307
    "308 Permanent Redirect",              // 308
    empty_sv,                              // 309
    empty_sv,                              // 310
    empty_sv,                              // 311
    empty_sv,                              // 312
    empty_sv,                              // 313
    empty_sv,                              // 314
    empty_sv,                              // 315
    empty_sv,                              // 316
    empty_sv,                              // 317
    empty_sv,                              // 318
    empty_sv,                              // 319
    empty_sv,                              // 320
    empty_sv,                              // 321
    empty_sv,                              // 322
    empty_sv,                              // 323
    empty_sv,                              // 324
    empty_sv,                              // 325
    empty_sv,                              // 326
    empty_sv,                              // 327
    empty_sv,                              // 328
    empty_sv,                              // 329
    empty_sv,                              // 330
    empty_sv,                              // 331
    empty_sv,                              // 332
    empty_sv,                              // 333
    empty_sv,                              // 334
    empty_sv,                              // 335
    empty_sv,                              // 336
    empty_sv,                              // 337
    empty_sv,                              // 338
    empty_sv,                              // 339
    empty_sv,                              // 340
    empty_sv,                              // 341
    empty_sv,                              // 342
    empty_sv,                              // 343
    empty_sv,                              // 344
    empty_sv,                              // 345
    empty_sv,                              // 346
    empty_sv,                              // 347
    empty_sv,                              // 348
    empty_sv,                              // 349
    empty_sv,                              // 350
    empty_sv,                              // 351
    empty_sv,                              // 352
    empty_sv,                              // 353
    empty_sv,                              // 354
    empty_sv,                              // 355
    empty_sv,                              // 356
    empty_sv,                              // 357
    empty_sv,                              // 358
    empty_sv,                              // 359
    empty_sv,                              // 360
    empty_sv,                              // 361
    empty_sv,                              // 362
    empty_sv,                              // 363
    empty_sv,                              // 364
    empty_sv,                              // 365
    empty_sv,                              // 366
    empty_sv,                              // 367
    empty_sv,                              // 368
    empty_sv,                              // 369
    empty_sv,                              // 370
    empty_sv,                              // 371
    empty_sv,                              // 372
    empty_sv,                              // 373
    empty_sv,                              // 374
    empty_sv,                              // 375
    empty_sv,                              // 376
    empty_sv,                              // 377
    empty_sv,                              // 378
    empty_sv,                              // 379
    empty_sv,                              // 380
    empty_sv,                              // 381
    empty_sv,                              // 382
    empty_sv,                              // 383
    empty_sv,                              // 384
    empty_sv,                              // 385
    empty_sv,                              // 386
    empty_sv,                              // 387
    empty_sv,                              // 388
    empty_sv,                              // 389
    empty_sv,                              // 390
    empty_sv,                              // 391
    empty_sv,                              // 392
    empty_sv,                              // 393
    empty_sv,                              // 394
    empty_sv,                              // 395
    empty_sv,                              // 396
    empty_sv,                              // 397
    empty_sv,                              // 398
    empty_sv,                              // 399
    "400 Bad Request",                     // 400
    "401 Unauthorized",                    // 401
    "402 Payment Required",                // 402
    "403 Forbidden",                       // 403
    "404 NotFound",                        // 404
    "405 Method Not Allowed",              // 405
    "406 Not Acceptable",                  // 406
    "407 Proxy Authentication Required",   // 407
    "408 Request Timeout",                 // 408
    "409 Conflict",                        // 409
    "410 Gone",                            // 410
    "411 Length Required",                 // 411
    "412 Precondition Failed",             // 412
    "413 Payload Too Large",               // 413
    "414 URI Too Long",                    // 414
    "415 Unsupported Media Type",          // 415
    "416 Range Not Satisfiable",           // 416
    "417 Expectation Failed",              // 417
    "418 I'm a teapot",                    // 418
    empty_sv,                              // 419
    empty_sv,                              // 420
    "421 Misdirected Request",             // 421
    "422 Unprocessable Entity",            // 422
    "423 Locked",                          // 423
    "424 Failed Dependency",               // 424
    "425 Too Early",                       // 425
    "426 Upgrade Required",                // 426
    empty_sv,                              // 427
    "428 Precondition Required",           // 428
    "429 Too Many Requests",               // 429
    empty_sv,                              // 430
    "431 Request Header Fields Too Large", // 431
    empty_sv,                              // 432
    empty_sv,                              // 433
    empty_sv,                              // 434
    empty_sv,                              // 435
    empty_sv,                              // 436
    empty_sv,                              // 437
    empty_sv,                              // 438
    empty_sv,                              // 439
    empty_sv,                              // 440
    empty_sv,                              // 441
    empty_sv,                              // 442
    empty_sv,                              // 443
    empty_sv,                              // 444
    empty_sv,                              // 445
    empty_sv,                              // 446
    empty_sv,                              // 447
    empty_sv,                              // 448
    empty_sv,                              // 449
    empty_sv,                              // 450
    "451 Unavailable For Legal Reasons",   // 451
    empty_sv,                              // 452
    empty_sv,                              // 453
    empty_sv,                              // 454
    empty_sv,                              // 455
    empty_sv,                              // 456
    empty_sv,                              // 457
    empty_sv,                              // 458
    empty_sv,                              // 459
    empty_sv,                              // 460
    empty_sv,                              // 461
    empty_sv,                              // 462
    empty_sv,                              // 463
    empty_sv,                              // 464
    empty_sv,                              // 465
    empty_sv,                              // 466
    empty_sv,                              // 467
    empty_sv,                              // 468
    empty_sv,                              // 469
    empty_sv,                              // 470
    empty_sv,                              // 471
    empty_sv,                              // 472
    empty_sv,                              // 473
    empty_sv,                              // 474
    empty_sv,                              // 475
    empty_sv,                              // 476
    empty_sv,                              // 477
    empty_sv,                              // 478
    empty_sv,                              // 479
    empty_sv,                              // 480
    empty_sv,                              // 481
    empty_sv,                              // 482
    empty_sv,                              // 483
    empty_sv,                              // 484
    empty_sv,                              // 485
    empty_sv,                              // 486
    empty_sv,                              // 487
    empty_sv,                              // 488
    empty_sv,                              // 489
    empty_sv,                              // 490
    empty_sv,                              // 491
    empty_sv,                              // 492
    empty_sv,                              // 493
    empty_sv,                              // 494
    empty_sv,                              // 495
    empty_sv,                              // 496
    empty_sv,                              // 497
    empty_sv,                              // 498
    empty_sv,                              // 499
    "500 Internal Server Error",           // 500
    "501 Not Implemented",                 // 501
    "502 Bad Gateway",                     // 502
    "503 Service Unavailable",             // 503
    "504 Gateway Timeout",                 // 504
    "505 HTTP Version Not Supported",      // 505
    "506 Variant Also Negotiates",         // 506
    "507 Insufficient Storage",            // 507
    "508 Loop Detected",                   // 508
    empty_sv,                              // 509
    "510 Not Extended",                    // 510
    "511 Network Authentication Required", // 511
};

// StatusCode
// ----------------------------------------------------------------------------

StatusCode iti::http::StatusCode::parse(int rawStatusCode) {
	if (rawStatusCode < isStatusCodeTable.size() &&
	    isStatusCodeTable[rawStatusCode]) {
		return StatusCode(static_cast<StatusCode::Value>(rawStatusCode));
	}

	throw std::logic_error(
	    fmt::format("unhandled raw status code {}", rawStatusCode));
}

bool iti::http::StatusCode::try_parse(int rawStatusCode,
                                      StatusCode &statusCode) {

	if (rawStatusCode < isStatusCodeTable.size() &&
	    isStatusCodeTable[rawStatusCode]) {
		statusCode = StatusCode(static_cast<StatusCode::Value>(rawStatusCode));
		return true;
	}

	return false;
}

std::string_view iti::http::StatusCode::str() const {
	if (int(value) < statusCodeStrTable.size()) {
		auto str = statusCodeStrTable[int(value)];
		if (!str.empty()) {
			return str;
		}
	}

	throw std::logic_error(fmt::format("unhandled status code {}", value));
}

#endif // ITI_LIB_HTTP_STATUSCODE_CPP

package org.just_somebody.pocket_pixel.core.networking

import org.just_somebody.pocket_pixel.core.Error

enum class NetworkError : Error
{
  TIMEOUT,
  AUTH_FAIL,
  CONFLICT,
  TOO_MANY_REQUESTS,
  NO_INTERNET,
  PAYLOAD_TOO_LARGE,
  SERVER_SIDE,
  UNKNOWN,
}
package org.just_somebody.pocket_pixel.core

sealed interface DataError : Error
{
  enum class Remote : DataError
  {
    REQUEST_TIMEOUT,
    TOO_MANY_REQUESTS,
    NO_INTERNET,
    SERVER_FAIL,
    SERIALIZATION,
    UNKnOWN
  }

  enum class Local : DataError
  {
    DISK_FULL,
    SEGFAULT
  }
}
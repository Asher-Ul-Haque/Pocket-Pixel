package just.somebody.templates.data

import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.network.NetworkResult
import just.somebody.templates.appModule.network.NetworkService
import just.somebody.templates.appModule.storage.InternalStorageManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.withContext
import kotlinx.serialization.builtins.MapSerializer
import kotlinx.serialization.builtins.serializer
import kotlinx.serialization.json.Json
import java.net.URLEncoder
import kotlin.math.max

interface BoxArtFetcher
{
  fun fetchBoxArt(GAME_NAME : String) : Flow<String?>
  fun deleteCache()
}

class DefaultBoxArtFetcher
(
  private val CACHE   : InternalStorageManager,
  private val NETWORK : NetworkService,
) : BoxArtFetcher
{
  private val BASE_URL         = "https://thumbnails.libretro.com/Nintendo%20-%20Game%20Boy/Named_Boxarts/"
  private val mappingCacheFile = "boxart_url_cache.json"

  private suspend fun listAvailableBoxArtFiles() : List<String> =
    withContext(Dispatchers.IO)
    {
      val result = NETWORK.get<String>(BASE_URL)
      when (result)
      {
        is NetworkResult.Success ->
          {
            val html    = result.data
            val regex   = Regex("""href="([^"]+\.png)"""")
            val matches = regex.findAll(html)
            matches.map { it.groupValues[1] }
              .distinct()
              .toList()
          }
        else -> emptyList()
      }
    }

  private suspend fun loadCache() : MutableMap<String, String> =
    withContext(Dispatchers.IO)
    {
      if (!CACHE.doesCacheExist(mappingCacheFile))
      {
        CACHE.cacheFile(mappingCacheFile, "{}".encodeToByteArray())
        return@withContext mutableMapOf()
      }
      val json = CACHE.readCache(mappingCacheFile)?.decodeToString() ?: return@withContext mutableMapOf()
      Json.decodeFromString(
        MapSerializer(String.serializer(), String.serializer()),
        json
      ).toMutableMap()
    }

  private suspend fun saveCache(DATA_CACHE : Map<String, String>) =
    withContext(Dispatchers.IO)
    {
      val json = Json.encodeToString(
        MapSerializer(String.serializer(), String.serializer()),
        DATA_CACHE
      )
      CACHE.cacheFile(mappingCacheFile, json.encodeToByteArray())
    }

  private fun normalize(NAME : String) : String
  {
    return NAME
      .lowercase()
      .replace(Regex("[^a-z0-9 ]"), " ")
      .replace(Regex("\\s+"), " ")
      .trim()
  }

  private fun stripExtension(NAME : String) : String =
    NAME.replace(Regex("\\.[a-z0-9]{1,5}$"), "")

  private fun urlEncode(S : String) : String
  {
    return URLEncoder.encode(S, Charsets.UTF_8.name())
      .replace("+", "%20")
  }

  private fun editDistance(A : String, B : String) : Int
  {
    val m  = A.length
    val n  = B.length
    val dp = Array(m + 1) { IntArray(n + 1) }

    for (i in 0..m) dp[i][0] = i
    for (j in 0..n) dp[0][j] = j

    for (i in 1..m)
    {
      for (j in 1..n)
      {
        val cost =
          if (A[i - 1] == B[j - 1]) 0
          else                      1
        dp[i][j] = minOf(
          dp[i - 1][j] + 1,
          dp[i][j - 1] + 1,
          dp[i - 1][j - 1] + cost
        )
      }
    }
    return dp[m][n]
  }

  private fun similarity(A : String, B : String) : Double
  {
    if (A.isEmpty() && B.isEmpty()) return 1.0
    if (A.isEmpty() || B.isEmpty()) return 0.0
    val distance = editDistance(A, B)
    return 1.0 - (distance.toDouble() / max(A.length, B.length))
  }

  private fun findClosest(
    TARGET     : String,
    CANDIDATES : List<String>,
    THRESHOLD  : Double
  ): String?
  {
    if (CANDIDATES.isEmpty()) return null

    val normalizedTarget = normalize(TARGET)

    for (candidate in CANDIDATES)
    {
      val normalizedCandidate = normalize(candidate)
      val sim                 = similarity(normalizedCandidate, normalizedTarget)
      if (sim >= THRESHOLD)
      {
        ForgeLogger.info("Fuzzy match: $candidate with score $sim")
        return candidate
      }
    }

    return null
  }

  override fun fetchBoxArt(GAME_NAME : String) : Flow<String?> = flow()
  {
    ForgeLogger.info("Fetching box art for: $GAME_NAME")

    val cacheJson      = loadCache()
    val cachedFileName = cacheJson[GAME_NAME]
    if (!cachedFileName.isNullOrBlank() && cachedFileName.endsWith(".png"))
    {
      val url = BASE_URL + cachedFileName
      ForgeLogger.info("Found in cache: $url")
      emit(url)
      return@flow
    }
    else ForgeLogger.info("Cache miss for: $GAME_NAME")

    val result = withContext(Dispatchers.Default)
    {
      val possibleExactName = stripExtension(GAME_NAME) + ".png"
      val exactUrl          = BASE_URL + urlEncode(possibleExactName)

      ForgeLogger.info("Checking exact match URL: $exactUrl")

      val headResult = withContext(Dispatchers.IO)
      { NETWORK.head(exactUrl) }

      if (headResult is NetworkResult.Success && headResult.data)
      {
        ForgeLogger.info("Exact match found: $possibleExactName")
        cacheJson[GAME_NAME] = possibleExactName
        saveCache(cacheJson)
        return@withContext exactUrl
      }

      ForgeLogger.info("Starting fuzzy search for: $GAME_NAME")

      val boxArtFiles = listAvailableBoxArtFiles()
      val candidates  = boxArtFiles.map { stripExtension(it) }

      val closest = findClosest(
        stripExtension(GAME_NAME),
        candidates,
        0.5
      )

      if (!closest.isNullOrBlank())
      {
        val closestFileName = boxArtFiles.firstOrNull{ stripExtension(it) == closest }
        if (!closestFileName.isNullOrBlank())
        {
          ForgeLogger.info("Fuzzy match found: $closestFileName")
          cacheJson[GAME_NAME] = closestFileName
          saveCache(cacheJson)
          return@withContext BASE_URL + closestFileName
        }
      }

      null
    }

    emit(result)
  }

  override fun deleteCache()
  {
    CACHE.cacheFile(mappingCacheFile, byteArrayOf())
    CACHE.deleteFile(mappingCacheFile)
    CACHE.deleteCache(mappingCacheFile)
  }
}

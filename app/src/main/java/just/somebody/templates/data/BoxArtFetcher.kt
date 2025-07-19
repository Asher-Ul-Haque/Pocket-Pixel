package just.somebody.templates.data

import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.HardwareManager
import just.somebody.templates.appModule.NetworkStatus
import just.somebody.templates.appModule.network.NetworkResult
import just.somebody.templates.appModule.network.NetworkService
import just.somebody.templates.appModule.storage.InternalStorageManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.collect
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.withContext
import kotlinx.serialization.builtins.MapSerializer
import kotlinx.serialization.builtins.nullable
import kotlinx.serialization.builtins.serializer
import kotlinx.serialization.json.Json
import java.net.URLEncoder
import kotlin.math.max

interface BoxArtFetcher {
  fun fetchBoxArt(GAME_NAME: String): Flow<String?>
  fun deleteCache()
}

class DefaultBoxArtFetcher(
  private val CACHE: InternalStorageManager,
  private val NETWORK: NetworkService,
) : BoxArtFetcher {
  private val BASE_URL = "https://thumbnails.libretro.com/Nintendo%20-%20Game%20Boy/Named_Boxarts/"
  private val mappingCacheFile = "boxart_url_cache.json"
  private var boxArtListCache: List<String>? = null

  private suspend fun listAvailableBoxArtFiles(): List<String> {
    boxArtListCache?.let { return it }

    val isConnected = App.appModule.hardwareManager.isConnectedToInternet.first()
    if (isConnected != NetworkStatus.Available) return emptyList()

    val result = NETWORK.get<String>(BASE_URL)
    val boxArts = when (result) {
      is NetworkResult.Success -> {
        Regex("""href="([^"]+\.png)"""")
          .findAll(result.data)
          .map { it.groupValues[1] }
          .distinct()
          .toList()
      }
      else -> emptyList()
    }

    boxArtListCache = boxArts
    return boxArts
  }

  private suspend fun loadCache(): MutableMap<String, String?> = withContext(Dispatchers.IO) {
    if (!CACHE.doesCacheExist(mappingCacheFile)) {
      CACHE.cacheFile(mappingCacheFile, "{}".encodeToByteArray())
      return@withContext mutableMapOf()
    }

    val json = CACHE.readCache(mappingCacheFile)?.decodeToString() ?: return@withContext mutableMapOf()
    return@withContext Json.decodeFromString(
      MapSerializer(String.serializer(), String.serializer().nullable),
      json
    ).toMutableMap()
  }

  private suspend fun saveCache(DATA_CACHE: Map<String, String?>) = withContext(Dispatchers.IO) {
    val json = Json.encodeToString(
      MapSerializer(String.serializer(), String.serializer().nullable),
      DATA_CACHE
    )
    CACHE.cacheFile(mappingCacheFile, json.encodeToByteArray())
  }

  private fun normalize(NAME: String): String =
    NAME.lowercase().replace(Regex("[^a-z0-9 ]"), " ").replace(Regex("\\s+"), " ").trim()

  private fun stripExtension(NAME: String): String =
    NAME.replace(Regex("\\.[a-z0-9]{1,5}$"), "")

  private fun urlEncode(S: String): String =
    URLEncoder.encode(S, Charsets.UTF_8.name()).replace("+", "%20")

  private fun editDistance(A: String, B: String): Int {
    val m = A.length
    val n = B.length
    val dp = Array(m + 1) { IntArray(n + 1) }

    for (i in 0..m) dp[i][0] = i
    for (j in 0..n) dp[0][j] = j

    for (i in 1..m) {
      for (j in 1..n) {
        val cost = if (A[i - 1] == B[j - 1]) 0 else 1
        dp[i][j] = minOf(dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost)
      }
    }
    return dp[m][n]
  }

  private fun similarity(A: String, B: String): Double {
    if (A.isEmpty() && B.isEmpty()) return 1.0
    if (A.isEmpty() || B.isEmpty()) return 0.0
    val distance = editDistance(A, B)
    return 1.0 - (distance.toDouble() / max(A.length, B.length))
  }

  private suspend fun findClosest(
    TARGET: String,
    CANDIDATES: List<String>,
    THRESHOLD: Double
  ): String? = withContext(Dispatchers.Default) {
    val normalizedTarget = normalize(TARGET)
    for (candidate in CANDIDATES) {
      val normalizedCandidate = normalize(candidate)
      val sim = similarity(normalizedCandidate, normalizedTarget)
      if (sim >= THRESHOLD) {
        ForgeLogger.info("Fuzzy match: $candidate with score $sim")
        return@withContext candidate
      }
    }
    return@withContext null
  }

  override fun fetchBoxArt(GAME_NAME: String): Flow<String?> = flow {
    ForgeLogger.info("Fetching box art for: $GAME_NAME")

    val cacheJson = loadCache()
    val cachedFileName = cacheJson[GAME_NAME]

    if (cachedFileName == "") {
      ForgeLogger.info("Previously not found (negative cache): $GAME_NAME")
      emit(null)
      return@flow
    }

    if (!cachedFileName.isNullOrBlank() && cachedFileName.endsWith(".png")) {
      val url = BASE_URL + cachedFileName
      ForgeLogger.info("Found in cache: $url")
      emit(url)
      return@flow
    }

    ForgeLogger.info("Cache miss for: $GAME_NAME")
    val possibleExactName = stripExtension(GAME_NAME) + ".png"
    val exactUrl = BASE_URL + urlEncode(possibleExactName)

    ForgeLogger.info("Checking exact match URL: $exactUrl")
    val headResult = withContext(Dispatchers.IO) { NETWORK.head(exactUrl) }

    if (headResult is NetworkResult.Success && headResult.data) {
      ForgeLogger.info("Exact match found: $possibleExactName")
      cacheJson[GAME_NAME] = possibleExactName
      saveCache(cacheJson)
      emit(exactUrl)
      return@flow
    }

    ForgeLogger.info("Starting fuzzy search for: $GAME_NAME")
    val boxArtFiles = listAvailableBoxArtFiles()
    val candidates = boxArtFiles.map { stripExtension(it) }

    val closest = findClosest(stripExtension(GAME_NAME), candidates, 0.5)
    if (!closest.isNullOrBlank()) {
      val closestFileName = boxArtFiles.firstOrNull { stripExtension(it) == closest }
      if (!closestFileName.isNullOrBlank()) {
        ForgeLogger.info("Fuzzy match found: $closestFileName")
        cacheJson[GAME_NAME] = closestFileName
        saveCache(cacheJson)
        emit(BASE_URL + closestFileName)
        return@flow
      }
    }

    ForgeLogger.info("No box art found for: $GAME_NAME")
    cacheJson[GAME_NAME] = "" // Negative cache
    saveCache(cacheJson)
    emit(null)
  }

  override fun deleteCache() {
    CACHE.cacheFile(mappingCacheFile, byteArrayOf())
    CACHE.deleteFile(mappingCacheFile)
    CACHE.deleteCache(mappingCacheFile)
  }
}

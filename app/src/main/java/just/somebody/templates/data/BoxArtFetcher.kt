package just.somebody.templates.data

import just.somebody.templates.App
import just.somebody.templates.appModule.ForgeLogger
import just.somebody.templates.appModule.NetworkStatus
import just.somebody.templates.appModule.network.NetworkResult
import just.somebody.templates.appModule.network.NetworkService
import just.somebody.templates.appModule.storage.InternalStorageManager
import just.somebody.templates.appModule.storage.LocalAssetManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.flow.flow
import kotlinx.coroutines.withContext
import kotlinx.serialization.builtins.MapSerializer
import kotlinx.serialization.builtins.nullable
import kotlinx.serialization.builtins.serializer
import kotlinx.serialization.json.Json
import java.net.URLEncoder
import kotlin.math.max

/**
 * Interface outlining structural queries to resolve, parse, and verify external promotional art content.
 */
interface BoxArtFetcher
{
  /**
   * Resolves a downloadable cover art resource path matching a targeted asset title context.
   *
   * @param GAME_NAME Structural item filename used as the key for index resolution.
   * @return Cold stream emission emitting the verified remote path layout or null if unresolved.
   */
  fun fetchBoxArt(GAME_NAME: String): Flow<String?>

  /** Flushes persistent local string mapping lookups from disk storage entirely. */
  fun deleteCache()
}

/**
 * Implementation layer coordinating image mapping lookups over network engines and internal cache directories.
 * Implements binary lookups paired with Levenshtein matrix calculations to provide fallback name tracking matching.
 *
 * @property CACHE Client layer directing operations for local private files and cache segments.
 * @property NETWORK Web engine broker executing GET calls against target remote server points.
 */
class DefaultBoxArtFetcher(
  private val CACHE   : InternalStorageManager,
  private val NETWORK : NetworkService) : BoxArtFetcher
{
  private val BASE_URLS: List<String> = listOf(
    "https://thumbnails.libretro.com/Nintendo%20-%20Game%20Boy/Named_Boxarts/",
    "https://thumbnails.libretro.com/Nintendo%20-%20Game%20Boy%20Color/Named_Boxarts/")

  private val mappingCacheFile    : String                            = "boxart_url_cache.json"
  private var boxArtListsCache    : MutableMap<String, List<String>>  = mutableMapOf()

  /**
   * Scans and scrapes a target platform URL directory layer to assemble an indexed manifest array of available files.
   *
   * @param BASE_URL Target remote folder route string to pull down.
   * @return Flat array matrix list of string file titles found inside the remote path.
   */
  private suspend fun listAvailableBoxArtFilesForUrl(BASE_URL: String): List<String>
  {
    // - - - Return from cache if the list for this URL is already available
    boxArtListsCache[BASE_URL]?.let { return it }

    val isConnected  = App.appModule.hardwareManager.isConnectedToInternet.first()
    if (isConnected != NetworkStatus.Available) return emptyList()

    val result  = NETWORK.get<String>(BASE_URL)
    val boxArts = when (result)
    {
      is NetworkResult.Success ->
      {
        Regex("""href="([^"]+\.png)"""")
          .findAll(result.data)
          .map { it.groupValues[1] }
          .distinct()
          .toList()
      }
      else -> emptyList()
    }

    // - - - Store the fetched list in the cache
    boxArtListsCache[BASE_URL] = boxArts
    return boxArts
  }

  /**
   * Extracts and reads out the persistent serialization mapping manifest cache table from local storage.
   *
   * @return A mutable key-value lookup structure mapping local item keys to resolved destination paths.
   */
  private suspend fun loadCache() : MutableMap<String, String?> =
    withContext(Dispatchers.IO)
    {
      if (!CACHE.doesCacheExist(mappingCacheFile))
      {
        CACHE.cacheFile(mappingCacheFile, "{}".encodeToByteArray())
        return@withContext mutableMapOf()
      }

      val json = CACHE.readCache(mappingCacheFile)?.decodeToString() ?: return@withContext mutableMapOf()
      return@withContext Json.decodeFromString(
        MapSerializer(String.serializer(),
                      String.serializer().nullable), json).toMutableMap()
    }

  /**
   * Encodes and flushes the key-value layout lookup matrix structural components back into local disk memory.
   *
   * @param DATA_CACHE Active key-value structure containing map transformations to commit.
   */
  private suspend fun saveCache(DATA_CACHE: Map<String, String?>) =
    withContext(Dispatchers.IO)
    {
      val json = Json.encodeToString(
        MapSerializer(String.serializer(), String.serializer().nullable),
        DATA_CACHE
                                    )
      CACHE.cacheFile(mappingCacheFile, json.encodeToByteArray())
    }

  /** Sanitizes unstructured file title characters down to alphanumeric spacing properties. */
  private fun normalize(NAME: String): String =
    NAME.lowercase().replace(Regex("[^a-z0-9 ]"), " ").replace(Regex("\\s+"), " ").trim()

  /** Truncates and drops trailing dot extension tags from a target path segment. */
  private fun stripExtension(NAME: String): String =
    NAME.replace(Regex("\\.[a-z0-9]{1,5}$"), "")

  /** Translates standard string character sequences into compliant safe URL Percent-Encoding components. */
  private fun urlEncode(S: String): String =
    URLEncoder.encode(S, Charsets.UTF_8.name()).replace("+", "%20")

  /**
   * Computes the absolute Levenshtein structural edit distance matrix mapping step variance between two strings.
   * Evaluates operations according to the formula:
   * * $$D(i, j) = \min \begin{cases} D(i-1, j) + 1 \\ D(i, j-1) + 1 \\ D(i-1, j-1) + \text{cost} \end{cases}$$
   *
   * @param A Base comparison token tracking character positions.
   * @param B Target comparison token tracking character positions.
   * @return Total transform integer calculations required to equalize structures.
   */
  private fun editDistance(A: String, B: String): Int
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
        dp[i][j] = minOf(dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost)
      }
    }
    return dp[m][n]
  }

  /**
   * Quantifies structural similarity ratios based on normalized Edit Distance evaluations.
   * * Returns a score mapping between $0.0$ (totally disparate strings) and $1.0$ (perfect structural symmetry).
   *
   * @param A Clean query phrase configuration string.
   * @param B Target catalog item mapping comparison layer.
   * @return Floating point percentage index tracking overall similarity bounds.
   */
  private fun similarity(A: String, B: String): Double
  {
    if (A.isEmpty() && B.isEmpty()) return 1.0
    if (A.isEmpty() || B.isEmpty()) return 0.0
    val distance = editDistance(A, B)
    return 1.0 - (distance.toDouble() / max(A.length, B.length))
  }

  /**
   * Asynchronously parses candidate listings inside a worker pool to extract the mathematically closest title match.
   *
   * @param TARGET Input request key sequence to resolve.
   * @param CANDIDATES Matrix array listing matching server choices extracted from folders.
   * @param THRESHOLD Lower boundary score floor limit where potential matches are rejected.
   * @return Optimal closest title text reference string found, or null if below evaluation floor properties.
   */
  private suspend fun findClosest(
    TARGET     : String,
    CANDIDATES : List<String>,
    THRESHOLD  : Double): String? =
    withContext(Dispatchers.Default)
    {
      val normalizedTarget    = normalize(TARGET)
      var bestMatch: String?  = null
      var bestScore           = THRESHOLD

      for (candidate in CANDIDATES)
      {
        val normalizedCandidate = normalize(candidate)
        val sim                 = similarity(normalizedCandidate, normalizedTarget)

        if (sim >= bestScore)
        {
          bestMatch = candidate
          bestScore = sim
        }
      }

      bestMatch?.let { ForgeLogger.info("Best fuzzy match: $it with score $bestScore") }
      return@withContext bestMatch
    }

  override fun fetchBoxArt(GAME_NAME: String): Flow<String?> =
    flow()
    {
      ForgeLogger.info("Fetching box art for: $GAME_NAME")

      // - - - Load the cache once at the start
      val cacheJson       = loadCache()
      val cachedUrl       = cacheJson[GAME_NAME]

      // - - - Check if a result is already in the cache
      if (cachedUrl == "")
      {
        ForgeLogger.info("Previously not found (negative cache): $GAME_NAME")
        emit(null)
        return@flow
      }

      if (!cachedUrl.isNullOrBlank())
      {
        ForgeLogger.info("Found in cache: $cachedUrl")
        emit(cachedUrl)
        return@flow
      }

      ForgeLogger.info("Cache miss for: $GAME_NAME")

      // - - - Loop through all the defined base URLs
      for (baseUrl in BASE_URLS)
      {
        ForgeLogger.info("Checking for box art in: $baseUrl")
        val possibleExactName = stripExtension(GAME_NAME) + ".png"

        val boxArtFiles = listAvailableBoxArtFilesForUrl(baseUrl)

        // - - - Check for exact match first
        val index = boxArtFiles.binarySearch(possibleExactName)
        if (index >= 0)
        {
          val foundFileName = boxArtFiles[index]
          val foundUrl      = baseUrl + foundFileName
          ForgeLogger.info("Exact match found via binary search: $foundUrl")
          
          val localUri = App.appModule.localAssetManager.getCachedAsset(
              url = foundUrl,
              fileName = "${normalize(GAME_NAME)}.png",
              category = LocalAssetManager.CATEGORY_BOXARTS
          )
          
          cacheJson[GAME_NAME] = localUri
          saveCache(cacheJson)
          emit(localUri)
          return@flow
        }

        // - - - If no exact match, try fuzzy search
        ForgeLogger.info("Starting fuzzy search for: $GAME_NAME in $baseUrl")
        val candidates = boxArtFiles.map { stripExtension(it) }

        val closest = findClosest(stripExtension(GAME_NAME), candidates, 0.65)
        if (!closest.isNullOrBlank())
        {
          val closestFileName = boxArtFiles.firstOrNull { stripExtension(it) == closest }
          if (!closestFileName.isNullOrBlank())
          {
            val foundUrl = baseUrl + closestFileName
            ForgeLogger.info("Fuzzy match found: $foundUrl")
            
            val localUri = App.appModule.localAssetManager.getCachedAsset(
                url = foundUrl,
                fileName = "${normalize(GAME_NAME)}.png",
                category = LocalAssetManager.CATEGORY_BOXARTS
            )

            cacheJson[GAME_NAME] = localUri
            saveCache(cacheJson)
            emit(localUri)
            return@flow
          }
        }
      }

      // - - - If the loop finishes without finding a box art, mark it as not found in the cache
      ForgeLogger.info("No box art found for: $GAME_NAME")
      cacheJson[GAME_NAME] = ""
      saveCache(cacheJson)
      emit(null)
    }

  override fun deleteCache()
  {
    CACHE.cacheFile(mappingCacheFile, byteArrayOf())
    CACHE.deleteFile(mappingCacheFile)
    CACHE.deleteCache(mappingCacheFile)
  }
}
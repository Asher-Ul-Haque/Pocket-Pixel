package just.somebody.templates.data

import just.somebody.templates.domain.Repository

class RepositoryImpl(private val API : Api) : Repository
{
  override suspend fun doSomething()
  {
    try { println("Repository did somethhing") }
    catch (e : Exception) { e.printStackTrace() }
  }
}
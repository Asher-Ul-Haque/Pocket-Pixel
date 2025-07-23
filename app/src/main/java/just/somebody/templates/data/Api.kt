package just.somebody.templates.data


interface Api
{
  fun         doSomething();
  suspend fun doSomethingElse();
}

class ApiImpl : Api
{
  override fun doSomething()
  { println("Something done") }

  override suspend fun doSomethingElse()
  { println("Something else done") }
}
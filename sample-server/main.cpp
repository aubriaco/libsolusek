#include <solusek/solusek.h>

static solusek::IServer *g_Server;

void interruptCallback(int sig)
{
	printf("Interrupt signal called.\n");
	g_Server->stop();
}

solusek::MResponse setCookieEndpoint(const void *sobj, solusek::MRequest& data)
{
    solusek::IServer *server = (solusek::IServer*)sobj;

    solusek::MResponse resp;
    solusek::MCookie cookie("test", "value");
    cookie.Path = "/";
    resp.Cookies.push_back(cookie);

    resp.Body = "Cookie set!";

    return resp;
}

solusek::MResponse getCookieEndpoint(const void *sobj, solusek::MRequest& data)
{
	  solusek::IServer *server = (solusek::IServer*)sobj;

		const solusek::MCookie c = data.getCookie("test");
		solusek::MResponse resp;
		if(!c.None)
		{
			resp.Body = "Cookie content: ";
			resp.Body += c.Value;
		}
	  else
		{
			resp.Body = "Cookie not found.";
		}

		return resp;
}

solusek::MResponse setSessionEndpoint(const void *sobj, solusek::MRequest& data)
{
	solusek::IServer *server = (solusek::IServer*)sobj;

	solusek::ISession* ses = server->startSession();
	ses->setStringValue("var1", "val1");
	ses->setStringValue("var2", "val2");

	return solusek::MResponse(200, "Session set.", ses->getID());
}

solusek::MResponse getSessionEndpoint(const void *sobj, solusek::MRequest& data)
{
	solusek::IServer *server = (solusek::IServer*)sobj;

	solusek::ISession* ses = server->getSession(data.SID);

	if(ses)
	{
		std::string body("Session found!\n\n");
		body += "var1 = " + ses->getStringValue("var1") + "\n";
		body += "var2 = " + ses->getStringValue("var2") + "\n";

		return solusek::MResponse(200, body);
	}
	return solusek::MResponse(200, "NO session found. Looking for ID: " + data.SID);
}

int main(int argc, char **argv)
{
	g_Server = solusek::createServer();

	g_Server->setInterruptCallback(interruptCallback);

	g_Server->setListenPort(8080);

	g_Server->addStaticDirectory("../static/solusek/", "solusek.com");
	g_Server->addStaticDirectory("../static/");

	g_Server->registerStaticIndex("index.html");

	g_Server->registerEndpoint(new solusek::MEndpoint("/api/cookie/set", setCookieEndpoint, "GET"));
	g_Server->registerEndpoint(new solusek::MEndpoint("/api/cookie/get", getCookieEndpoint, "GET"));
	g_Server->registerEndpoint(new solusek::MEndpoint("/api/session/set", setSessionEndpoint, "GET"));
	g_Server->registerEndpoint(new solusek::MEndpoint("/api/session/get", getSessionEndpoint, "GET"));

	g_Server->setThreadLimit(50);

	g_Server->run();

	g_Server->dispose();

	return 0;
}

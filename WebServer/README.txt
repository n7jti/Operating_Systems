Name: Charles (Alan) Ludwig
E-Mail: al@theludwigfamily.com
UWID: alanlu

My servers are composed of small number of modules that are specific to each scenario (threaded vs. event) and a larger number of common modules.  The modules provide a layered abstraction of the functional tasks of a web server.  The main program and the sever components are unique to the scenario while all lower level components (sockets, requests state, and web content) are shared.  There is no 3rd party code in the server. However, to be honest the server started from an example of an "echo service" which provided the initial dozen or so lines of code that read and wrote to sockets.  However, it would be difficult to point to a single line of code in the project that remains from that early start.  

	     +---------------+
	     |  main program |
	     +---------------+
		     |
	     +---------------+
	     |    server     |
	     +---------------+
                     |
      +--------------+-----------------+
      |              |                 |
+----------+   +--------------+  +-------------+
|  sockets |   |request state |  | web content |
+----------+   +--------------+  +-------------+

Threaded Server specific modules:

Main Program (tmain.cpp) : main entry point for the program. It parses the arguments and prints usage information.  If all is well, it creates an instance of the server and calls Run.  Additionally it also contains the signal handlers.  Tmain.cpp also hosts a single global variable that is set in the signal handler associated with Ctrl-C and is used to tear down the server in a controlled manor. 

Server (tserv.cpp/h) : The thread based server.  This module contains the server loop and a static method of the contained class is the thread procedure.  When a request comes in, it is captured and a thread is created to service the request.  The state variable used to process the request is passed as the parameter to the thread proc.  The main server loop terminates when a global variable controlled by a signal handler. 

Event Server Specific Modules

Main Program (emain.cpp/h) : This is main entry point for the event based server.  It is nearly exactly the same as the threaded version.  The primary difference is that this main uses CEServ calls as its server, while the threaded server uses CTServ.

Server (eserv.cpp/h) : The event based server. This version of the server uses the poll() system call to block until the various flavors of I/O are ready.  When the poll() call unblocks a flag is set in an array of structures.  From this structure, the associated file descriptor is read and then used as the key to an STL map that returns the associated state for the http request.  From there the internal state drives the processing. The server does teardown when the signal handler sets a global variable and Run() falls out the bottom. 

Common Modules:

Server (serv.cpp) :  This is the base class for the web server. It has a pure virtual method Run() which is overloaded by the derived classes.  This method contains the main sever loop and blocks until the server is torn down.  The base class also contains the functionality for loading the in-memory cache for the web content and storing it in an STL map that maps the request string to the content. Methods for loading the cache are in this base class.

Web Content (content.cpp/h) : The content class is the in-memory store for a single piece of web content.  There are methods for reading in the content from a file descriptor and for simple accessors to get the data.  a number of these are stored in an STL map in the server class. The web content includes not just the content pages but a small number of error pages as well.  

Sockets (csock.cpp/h) : This class contains the RAII wrappers for the windows sockets.  The main purpose of these classes is to wrap the socket objects in a slightly higher level abstraction and to automatically manage the teardown of the sockets when we are through.  The constructor for the sockets class take a flag which is used when constructing the sockets and allows a client of the classes to create blocking or non-blocking versions of the sockets.

Web Request (request.cpp/h) : This class is where the magic happens with respect to the state for web request.  It contains the state variables and the state engine for processing a request.  It also contains the algorithyms for parsing a request and building the response.  


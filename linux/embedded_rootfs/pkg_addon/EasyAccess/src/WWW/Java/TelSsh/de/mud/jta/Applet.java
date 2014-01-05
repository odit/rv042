package de.mud.jta;

import de.mud.jta.event.AppletRequest;
import de.mud.jta.event.FocusStatusListener;
import de.mud.jta.event.OnlineStatusListener;
import de.mud.jta.event.ReturnFocusRequest;
import de.mud.jta.event.SocketRequest;
import de.mud.jta.event.SoundListener;

import java.awt.*;
import java.awt.datatransfer.Clipboard;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.URL;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Properties;
import java.util.Vector;
import java.util.StringTokenizer;

public class Applet extends java.applet.Applet {

  private final static int debug = 0;

  private String frameTitle = null;
  private java.awt.Container appletFrame;

  /** holds the defaults */
  private Properties options = new Properties();

  /** hold the common part of the jta */
  private Common pluginLoader;

  /** hold the host and port for our connection */
  private String tunnelServer, host, port, session, timeOut, service, https_port;  //Rain

  /** disconnect on leave, this is to force applets to break the connection */
  private boolean disconnect = true;
  /** connect on startup, this is to force applets to connect on detach */
  private boolean connect = false;
  /** close the window (if it exists) after the connection is lost */
  private boolean disconnectCloseWindow = true;

  private Plugin focussedPlugin;
  private Clipboard clipboard;
  private boolean online = false;

  private HelpFrame helpFrame = null;

  /**
   * Read all parameters from the applet configuration and
   * do initializations for the plugins and the applet.
   */
  public void init() {
    if (debug > 0) System.err.println("Applet: init()");
    if (pluginLoader == null) {
      try {
        options.load(Applet.class.getResourceAsStream("/de/mud/jta/default.conf"));
      } catch (Exception e) {
          System.err.println("jta: cannot load default.conf");
      }

      String value;

      // try to load the local configuration and merge it with the defaults
      if ((value = getParameter("config")) != null) {
        Properties appletParams = new Properties();
        URL url = null;
        try {
          url = new URL(value);
        } catch (Exception e) {
          try {
            url = new URL(getCodeBase() + value);
          } catch (Exception ce) {
            System.err.println("jta: could not find config file: " + ce);
          }
        }

        if (url != null) {
          try {
            appletParams.load(Applet.class.getResourceAsStream("/de/mud/jta/" + value));
            Enumeration ape = appletParams.keys();
            while (ape.hasMoreElements()) {
              String key = (String) ape.nextElement();
              options.put(key, appletParams.getProperty(key));
            }
          } catch (Exception e) {
            try {
              appletParams.load(url.openStream());
              Enumeration ape = appletParams.keys();
              while (ape.hasMoreElements()) {
                String key = (String) ape.nextElement();
                options.put(key, appletParams.getProperty(key));
              }
            } catch (Exception e2) {
              System.err.println("jta: could not load config file: " + e2);
            }
          }
        }
      }

      // see if there are parameters in the html to override properties
      parameterOverride(options);

      // configure the application and load all plugins
      pluginLoader = new Common(options);

/*
      host = options.getProperty("Socket.host");
      if (host == null)
        host = getCodeBase().getHost();
      port = options.getProperty("Socket.port");
      if (port == null)
        port = "23";
*/
      host = getParameter("host");
      if (host == null)
        host = getCodeBase().getHost();

	  ParseHostName(host);
	  
	  tunnelServer = getParameter("tunnel");
	  if(tunnelServer == null)
        tunnelServer = getCodeBase().getHost();

      session = getParameter("session");
      if(session == null)
        stop();

      timeOut = getParameter("timeout");
      if(timeOut == null)
        de.mud.jta.plugin.Constants.timeOut = new Integer("0");
      else
        de.mud.jta.plugin.Constants.timeOut = new Integer(timeOut);
	//Rain ->
      https_port = getParameter("https_port");
      if(https_port == null)
        https_port = "443";
//<-Rain
      if(port == null)
      {
		port = getParameter("port");
		if(port == null) 
		  {
		    if(pluginLoader.getPlugins().containsKey("SSH")) 
		      port = "22";
		    else
		      port = "23";
		  }
	  }

      if(pluginLoader.getPlugins().containsKey("SSH"))
        service = "SSH";
      else
        service = "TELNET";

      if ((new Boolean(options.getProperty("Applet.connect"))
              .booleanValue()))
        connect = true;
      if (!(new Boolean(options.getProperty("Applet.disconnect"))
              .booleanValue()))
        disconnect = false;

      if (!(new Boolean(options.getProperty("Applet.disconnect.closeWindow"))
              .booleanValue()))
        disconnectCloseWindow = false;

      frameTitle = options.getProperty("Applet.detach.title");

      if ((new Boolean(options.getProperty("Applet.detach"))).booleanValue()) {
        if (frameTitle == null)
          appletFrame = new Frame("jta: " + host + port);
        else
          appletFrame = new Frame(frameTitle);
      } else
        appletFrame = this;

      appletFrame.setLayout(new BorderLayout());

      Hashtable componentList = pluginLoader.getComponents();
      Enumeration names = componentList.keys();
      while (names.hasMoreElements()) {
        String name = (String) names.nextElement();
        Component c = (Component) componentList.get(name);
        if ((value = options.getProperty("layout." + name)) != null) {
          appletFrame.add(value, c);
        } else {
          System.err.println("jta: no layout property set for '" + name + "'");
          System.err.println("jta: ignoring '" + name + "'");
        }
      }

      pluginLoader.registerPluginListener(new SoundListener() {
        public void playSound(URL audioClip) {
          Applet.this.getAudioClip(audioClip).play();
        }
      });

      pluginLoader.broadcast(new AppletRequest(this));
      if (appletFrame != this) {
        final String startText = options.getProperty("Applet.detach.startText");
        final String stopText = options.getProperty("Applet.detach.stopText");
        final Button close = new Button();

        // set up the clipboard
        try {
          clipboard = appletFrame.getToolkit().getSystemClipboard();
          System.err.println("Applet: acquired system clipboard: " + clipboard);
        } catch (Exception e) {
          System.err.println("Applet: system clipboard access denied: " +
                             ((e instanceof InvocationTargetException) ?
                              ((InvocationTargetException) e).getTargetException() : e));
          // e.printStackTrace();
        } finally {
          if (clipboard == null) {
            System.err.println("Applet: copy & paste only within the JTA");
            clipboard = new Clipboard("de.mud.jta.Main");
          }
        }

        if ((new Boolean(options.getProperty("Applet.detach.immediately"))
                .booleanValue())) {
          if ((new Boolean(options.getProperty("Applet.detach.fullscreen"))
                  .booleanValue()))
            ((Frame) appletFrame)
                    .setSize(appletFrame.getToolkit().getScreenSize());
          else
            ((Frame) appletFrame).pack();

          ((Frame) appletFrame).show();
		  //Rain-->
          //pluginLoader.broadcast(new SocketRequest(tunnelServer, session, host, Integer.parseInt(port), service));
		  pluginLoader.broadcast(new SocketRequest(tunnelServer, session, host, Integer.parseInt(port), service, Integer.parseInt(https_port)));
		  //<--Rain
          pluginLoader.broadcast(new ReturnFocusRequest());
          close.setLabel(startText != null ? stopText : "Disconnect");
        } else
          close.setLabel(startText != null ? startText : "Connect");

        close.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent evt) {
            if (((Frame) appletFrame).isVisible()) {
              pluginLoader.broadcast(new SocketRequest());
              ((Frame) appletFrame).setVisible(false);
              close.setLabel(startText != null ? startText : "Connect");
            } else {
              if (frameTitle == null)
                ((Frame) appletFrame)
                        .setTitle("jta: " + host + port);
              if ((new Boolean(options.getProperty("Applet.detach.fullscreen"))
                      .booleanValue()))
                ((Frame) appletFrame)
                        .setSize(appletFrame.getToolkit().getScreenSize());
              else
                ((Frame) appletFrame).pack();
              ((Frame) appletFrame).show();
              getAppletContext().showStatus("Trying " + host + " " + port + " ...");
			  //Rain-->
              //pluginLoader.broadcast(new SocketRequest(tunnelServer, session, host,
                                                       //Integer.parseInt(port), service));
				pluginLoader.broadcast(new SocketRequest(tunnelServer, session, host,
                                                       Integer.parseInt(port), service, Integer.parseInt(https_port)));
			  //<--Rain
              pluginLoader.broadcast(new ReturnFocusRequest());
              close.setLabel(stopText != null ? stopText : "Disconnect");
            }
          }
        });

        setLayout(new BorderLayout());
        add("Center", close);

        // add a menu bar
        MenuBar mb = new MenuBar();
        Menu file = new Menu("File");
        file.setShortcut(new MenuShortcut(KeyEvent.VK_F, true));
        MenuItem tmp;
        file.add(tmp = new MenuItem("Connect"));
        tmp.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent evt) {
		  //Rain-->
            //pluginLoader.broadcast(new SocketRequest(tunnelServer, session, host,
              //                                       Integer.parseInt(port), service));
			pluginLoader.broadcast(new SocketRequest(tunnelServer, session, host,
                                                     Integer.parseInt(port), service, Integer.parseInt(https_port)));
          //<--Rain
		  }
        });
        file.add(tmp = new MenuItem("Disconnect"));
        tmp.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent evt) {
            pluginLoader.broadcast(new SocketRequest());
          }
        });
        file.add(new MenuItem("-"));
        file.add(tmp = new MenuItem("Print"));
        tmp.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent evt) {
            if (pluginLoader.getComponents().get("Terminal") != null) {
              PrintJob printJob =
                      appletFrame.getToolkit()
                      .getPrintJob((Frame) appletFrame, "Terminal", null);
              ((Component) pluginLoader.getComponents().get("Terminal"))
                      .print(printJob.getGraphics());
              printJob.end();
            }
          }
        });
        file.add(new MenuItem("-"));
        file.add(tmp = new MenuItem("Exit"));
        tmp.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent evt) {
            ((Frame) appletFrame).setVisible(false);
            pluginLoader.broadcast(new SocketRequest());
            close.setLabel(startText != null ? startText : "Connect");
          }
        });
        mb.add(file);

        Menu edit = new Menu("Edit");
        edit.setShortcut(new MenuShortcut(KeyEvent.VK_E, true));
        edit.add(tmp = new MenuItem("Copy"));
        tmp.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent evt) {
            if (debug > 2)
              System.err.println("Applet: copy: " + focussedPlugin);
            if (focussedPlugin instanceof VisualTransferPlugin)
              ((VisualTransferPlugin) focussedPlugin).copy(clipboard);
          }
        });
        edit.add(tmp = new MenuItem("Paste"));
        tmp.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent evt) {
            if (debug > 2)
              System.err.println("Applet: paste: " + focussedPlugin);
            if (focussedPlugin instanceof VisualTransferPlugin)
              ((VisualTransferPlugin) focussedPlugin).paste(clipboard);
          }
        });
        mb.add(edit);

        Hashtable menuList = pluginLoader.getMenus();
        names = menuList.keys();
        while (names.hasMoreElements()) {
          String name = (String) names.nextElement();
          mb.add((Menu) menuList.get(name));
        }

        Menu help = new Menu("Help");
        help.setShortcut(new MenuShortcut(KeyEvent.VK_HELP, true));
        help.add(tmp = new MenuItem("General"));
        tmp.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent e) {
            if(helpFrame == null) {
              helpFrame = new HelpFrame(options.getProperty("Help.url"));
            }
            helpFrame.setVisible(true);
          }
        });
        mb.setHelpMenu(help);

        // only add the menubar if the property is true
        if ((new Boolean(options.getProperty("Applet.detach.menuBar"))
                .booleanValue()))
          ((Frame) appletFrame).setMenuBar(mb);

        // add window closing event handler
        try {
          ((Frame) appletFrame).addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent evt) {
              pluginLoader.broadcast(new SocketRequest());
              ((Frame) appletFrame).setVisible(false);
              close.setLabel(startText != null ? startText : "Connect");
            }
          });
        } catch (Exception e) {
          System.err.println("Applet: could not set up Window event listener");
          System.err.println("Applet: you will not be able to close it");
        }

        pluginLoader.registerPluginListener(new OnlineStatusListener() {
          public void online() {
            if (debug > 0) System.err.println("Terminal: online");
            online = true;
            if (((Frame) appletFrame).isVisible() == false)
              ((Frame) appletFrame).setVisible(true);
          }

          public void offline() {
            if (debug > 0) System.err.println("Terminal: offline");
            online = false;
            if (disconnectCloseWindow) {
              ((Frame) appletFrame).setVisible(false);
              close.setLabel(startText != null ? startText : "Connect");
            }
          }
        });

        // register a focus status listener, so we know when a plugin got focus
        pluginLoader.registerPluginListener(new FocusStatusListener() {
          public void pluginGainedFocus(Plugin plugin) {
            if (Applet.debug > 0)
              System.err.println("Applet: " + plugin + " got focus");
            focussedPlugin = plugin;
          }

          public void pluginLostFocus(Plugin plugin) {
            // we ignore the lost focus
            if (Applet.debug > 0)
              System.err.println("Applet: " + plugin + " lost focus");
          }
        });

      } else
      // if we have no external frame use this online status listener
        pluginLoader.registerPluginListener(new OnlineStatusListener() {
          public void online() {
            if (debug > 0) System.err.println("Terminal: online");
            online = true;
          }

          public void offline() {
            if (debug > 0) System.err.println("Terminal: offline");
            online = false;
          }
        });
    }
  }

  /**
   * Start the applet. Connect to the remote host.
   */
  public void start() {
    if (!online && (appletFrame == this || connect)) {
      if (debug > 0) System.err.println("start(" + host + ", " + port + ")");
      getAppletContext().showStatus("Trying " + host + " " + port + " ...");
	  //Rain -->
      //pluginLoader.broadcast(new SocketRequest(tunnelServer, session, host, Integer.parseInt(port), service));
	  pluginLoader.broadcast(new SocketRequest(tunnelServer, session, host, Integer.parseInt(port), service, Integer.parseInt(https_port)));
	  //<-- Rain
      pluginLoader.broadcast(new ReturnFocusRequest());
    }
  }

  /**
   * Stop the applet and disconnect.
   */
  public void stop() {
    if (online && disconnect) {
      if (debug > 0) System.err.println("stop()");
      pluginLoader.broadcast(new SocketRequest());
    }
  }

  /**
   * Override any properties that are found in the configuration files
   * with possible values found as applet parameters.
   * @param options the loaded configuration file properties
   */
  private void parameterOverride(Properties options) {
    Enumeration e = options.keys();
    while (e.hasMoreElements()) {
      String key = (String) e.nextElement(), value = getParameter(key);
      if (value != null) {
        System.out.println("Applet: overriding value of " + key + " with " + value);
        // options.setProperty(key, value);
        options.put(key, value);
      }
    }
  }
  
    /** Parse the host name to find that port number is passed along with host name */
  private void ParseHostName(String hostname){
	  StringTokenizer st = new StringTokenizer(hostname, ":", false);
     if (st.hasMoreTokens()) {
         host = st.nextToken();
		 if (st.hasMoreTokens()){
			 port = st.nextToken();
		 }
		
     }
	  
  }


  
}

package com.menlo;

public class MenloMgr extends Thread 
{
    static int MAX_ROUTES = 16;
    static int MAX_NAME_SERVER = 3;

    private native int  addRoute (String route); 
    private native int  getIfStatus ();
    private native void cleanup();
    private native int waitForPPP();

    private native int setDNS(String primary, String secondary);
    private native int cleanupDNS();


    private native int setWins(String primary, String secondary);
    private native int cleanupWins();

    String[] routes;
    String[] dnsServer;
    String[] winsServer;

    MenloSSLApplet applet = null;
    int retVal;

    int dnsStatus = 0;
    int winsStatus = 0;
    
    static 
      {
        System.loadLibrary("MenloDialer");
      }
    
    public MenloMgr(String[] routes, String[] dns, String[] wins)
      {
        super("MenloMgr");

        int i;
        this.routes = new String[MenloMgr.MAX_ROUTES];

        for(i=0; ((routes[i] != null) && (i < MenloMgr.MAX_ROUTES)); i++)
          {
            this.routes[i] = new String(routes[i]);
          }
        this.routes[i] = null;

        if(dns != null)
          {
            dnsServer = new String[MenloMgr.MAX_NAME_SERVER];
            for(i=0; ((dns[i] != null) && (i < MenloMgr.MAX_ROUTES)); i++)
              {
                dnsServer[i] = new String(dns[i]);
              }
            dnsServer[i] = null;
          }
        else
          dnsServer = null;

        if(wins != null)
          {
            winsServer = new String[MenloMgr.MAX_NAME_SERVER];
            for(i=0; ((wins[i] != null) && (i < MenloMgr.MAX_ROUTES)); i++)
              {
                winsServer[i] = new String(wins[i]);
              }
            winsServer[i] = null;
          }
        else
          winsServer = null;
      }

    public void setContainer(MenloSSLApplet applet)
      {
        this.applet = applet;
      }

    public void run()
      {
        int i;
        retVal = waitForPPP();
        if(retVal < 0)
          {
            System.out.println("Mgr: PPP Interface failure");
            applet.notifyExitStatus(retVal);
            return;
          }

        applet.setInfoStatus("Connection established.");
        try {
            wait(1);
        }catch(Exception e) {};

        applet.setInfoStatus("Adding routes..");
        for(i=0; ((routes[i] != null) && (i < MenloMgr.MAX_ROUTES)); i++)
          {
            retVal = addRoute(routes[i]);
            if(retVal < 0)
              applet.setWarnStatus("Route addition failed: " + routes[i]);
            else
              applet.setInfoStatus("Route added: " + routes[i]);
          }

        
        if(dnsServer != null)
          {
            applet.setInfoStatus("Setting DNS configuration..");
            System.out.println("Primary DNS: " + dnsServer[0] + ", Secondary DNS:" + dnsServer[1]);
            dnsStatus = setDNS(dnsServer[0], dnsServer[1]);
            if(dnsStatus != 1)
              applet.setWarnStatus("DNS Configuration fiailed. Reboot for safety");
            else
            applet.setInfoStatus("VPN DNS configuration enabled.");
          }

        if(winsServer != null)
          {
            applet.setInfoStatus("Setting WINS configuration..");
            winsStatus = setWins(winsServer[0], winsServer[1]);
            if(winsStatus != 1)
              applet.setWarnStatus("WINS Configuration fiailed.");
            else
            applet.setInfoStatus("VPN WINS configuration enabled.");
          }

        applet.setInfoStatus("Connected to:  " + applet.getDocumentBase().getHost());
      }

    public void stopMgr()
      {
        cleanup();

        if(dnsStatus == 1)
          cleanupDNS();

        if(winsStatus == 1)
          cleanupWins();
      }

    public int getStatus()
      {
        return getIfStatus();
      }
}

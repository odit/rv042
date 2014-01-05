package com.menlo;

import javax.swing.*;
import java.awt.*;
import java.util.*;
import java.awt.event.*;

public class MenloDialer extends Thread 
{
    private native int  dialConnection (String[] args, int timeOut); 
    private native int  closeConnection (String[] args);

    String[] param;
    MenloSSLApplet applet = null;
    int retVal;
    int timeOut = 0;
    boolean stopped = false;
    
    static 
      {
        System.loadLibrary("MenloDialer");
      } 
    
    public MenloDialer(String[] args) 
      { 
        super("MenloDialer");
        param = new String[2];
        param[0] = new String(args[0]);
        param[1] = new String(args[1]);
      }

    public void setContainer(MenloSSLApplet applet)
      {
        this.applet = applet;
      }

    public void setTimeout(int timeOut)
      {
        this.timeOut = timeOut;
      }

    public void run()
      {
        retVal = dialConnection (param, timeOut);

        if((applet != null) && (stopped != true))
          {
            applet.notifyExitStatus(retVal);
          }
        //System.out.println("Dial Connection completed...");
        return ;
      }

    public void stopConnection()
      {
        if(stopped)
          return;

        retVal = closeConnection(param);
        stopped = true;
      }
}

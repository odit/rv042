package com.code;

import java.awt.Color;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.BorderLayout;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JLabel;
import javax.swing.JPanel; 
import javax.swing.JApplet;
import javax.swing.JButton;
import javax.swing.BorderFactory; 
import javax.swing.border.EtchedBorder;

import netscape.javascript.JSObject;

class CustomLabel extends JLabel 
{
    static final long serialVersionUID = 0;
    String statusLog[];
    static int MAX_LOG_COUNT = 32;
    int logCount = 0;

    public CustomLabel(String label)
      {
        super(label);
        statusLog = new String[MAX_LOG_COUNT];
      }

    public CustomLabel(String label, int rgb)
      {
        super(label, rgb);
        statusLog = new String[MAX_LOG_COUNT];
      }

    public void setStatusText(String text)
      {
        if(logCount < MAX_LOG_COUNT)
          statusLog[logCount++] = new String(text);

        setText(text);
        setToolTipText(getLogText());
      }

    public String getLogText()
      {
        String str = new String("<html> <hr> <p> Status Log </p> <hr>\n");
        String str1;
        int i = 0;
        while((i< MAX_LOG_COUNT) && (statusLog[i] != null))
          {
            str1 = str;
            str = str1.concat("<p>" + statusLog[i++] + "</p>\n");
          }
        return str.concat("</html>\n");
      }
}

public class SSLApplet extends JApplet implements ActionListener
  {
    static final long serialVersionUID = 0;
    JButton jbConnect;
    JButton jbDisconnect;
    JLabel label;
    CustomLabel status;
    Dialer dialer;
    Mgr mgr;
    String[] args;
    String strOperatingSystem;
    String strConnectionStatus, strServerIP;
    boolean isFullTunnelEnabled = false;
    int timeOut = 0;

    String paramName;
    JPanel panel;
    Container mainPanel;
    int exitStatus = 0;
    int stopped = 0;

    private String normalizeAddress(String strIPAddress)
      {
	if(strIPAddress.startsWith("[") && strIPAddress.endsWith("]"))
	  {
  	    strIPAddress = 
	  			strIPAddress.substring(strIPAddress.indexOf("[")+1, strIPAddress.indexOf("]"));
	  }
	return strIPAddress;
      }
	  
    public void init()
      {
        args = new String[3];

        args[0] = getParameter("host");
        if(args[0] == null || 0 == args[0].trim().length())
          args[0] = getDocumentBase().getHost();

		args[1] = getParameter("session");
		args[2] = getParameter("PORT");		/*chihmou 20071022 */
        if(getParameter("timeout") != null)
          {
            Integer t = new Integer(getParameter("timeout"));
            timeOut = t.intValue();
          }

        jbConnect = new JButton("Connect");
        jbDisconnect = new JButton("Disconnect");
        label = new JLabel(" Virtual Passage", JLabel.CENTER);
        status = new CustomLabel("Click Connect to start");

        status.setToolTipText(status.getLogText());

        strOperatingSystem = System.getProperty("os.name");
		isFullTunnelEnabled = (getParameter("full_tunnel") != null);
      }


    public void start()
      {
        mainPanel = getContentPane();
        mainPanel.setLayout(new BorderLayout(10, 20));

        label.setMinimumSize(new Dimension(50, 50));
        label.setPreferredSize(new Dimension(50, 50));
        mainPanel.add(label, BorderLayout.NORTH);

        panel = new JPanel();
        panel.setBorder(BorderFactory.createEtchedBorder(EtchedBorder.LOWERED));

        panel.setLayout(new GridLayout(1,2,10,10));
        panel.add(jbConnect);
        panel.add(jbDisconnect);

        mainPanel.add(panel, BorderLayout.CENTER);

        status.setBorder(BorderFactory.createEtchedBorder(EtchedBorder.RAISED));
        status.setForeground(Color.blue);
        mainPanel.add(status, BorderLayout.SOUTH);

        jbConnect.addActionListener( this ); 
        jbDisconnect.addActionListener( this );

        jbConnect.setActionCommand( "c" );
        jbDisconnect.setActionCommand( "d" );  

        jbDisconnect.setEnabled(false);

        connect();
      }

    /* 
     * This method instantiates the Dialer and Manager classes for setting up
     * PPP connection and spawns a thread for each of them.
     */
    private void connect()
      {
        strConnectionStatus = null;
		exitStatus = 0;
		stopped = 0;
		
        jbConnect.setEnabled(false);
        
        Debug.initializeLogger();
        
        args[0] = normalizeAddress(args[0]);
        
        Debug.logMessage("Client OS : "+strOperatingSystem);
        Debug.logMessage("Server Address:" + args[0]);
        Debug.logMessage("Session ID:" + args[1]);
	Debug.logMessage("PORT:" + args[2]);		/* chihmou  20071022 */

        dialer = 
        new Dialer(args, Debug.getFileName(), Debug.getDebugEnabled(), isFullTunnelEnabled);

		strServerIP = dialer.getResolvedHost();
		
		Debug.logMessage("Resolved host address : "+strServerIP);
        if(strServerIP == null || strServerIP.trim().length() == 0)
          {
          	notifyExitStatus(-2);
          	return ;
          }

        mgr = new Mgr(strServerIP.trim(),strOperatingSystem, isFullTunnelEnabled);

        mgr.setContainer(this);

		if(!mgr.canPPPConnect())
		  {
          	notifyExitStatus(-3);
	        Debug.logMessage("Aborting PPP connection setup\n");
          	return ;
		  }

        Debug.logMessage("About to invoke Manager thread");
        mgr.start();

        dialer.setContainer(this);
        dialer.setTimeout(timeOut);

        status.setStatusText("Connecting ...");
        Debug.logMessage("About to invoke Dialer thread");
        dialer.start();

        Debug.logMessage("Dialer & Manager threads invoked");

	exitStatus = 0;
        stopped = 0;
      }

    private void closeAppletWindow()
      {
      	Debug.closeLogger();
        JSObject win = (JSObject) JSObject.getWindow(this);
        win.eval("window.opener.closeInstallWindow();");
      }

    /* This method attempts to stop the threads spawned in connect */
    private void disconnect()
      {
        jbDisconnect.setEnabled(false);
        status.setStatusText("Disconnecting...");
	terminateThreads();
	closeAppletWindow();
        status.setForeground(Color.blue);
        status.setStatusText("Disconnected");
        jbConnect.setEnabled(true);
      }

    public void actionPerformed( ActionEvent e )
      {
        String command = e.getActionCommand();

        if (command.equals( "c" )) 
          connect();
        else if (command.equals( "d" )) 
          disconnect();
      }

    public void terminateThreads()
      {
        if(stopped == 1)
          return;

        stopped = 1;

        if(mgr != null)
          {
	          mgr.stopMgr();
	
	        try 
	          {
	          	Debug.logMessage("Waiting for Manager thread to terminate\n");
		        mgr.join();
		        Debug.logMessage("Manager thread terminated\n");
		        mgr = null;
			  }
	        catch(Exception objE) 
	          {
	        	Debug.logMessage(objE.toString());
	          }
          }

        if(dialer != null)
          {
	          dialer.stopConnection();
			try
			  {	
	         	Debug.logMessage("Waiting for Dialer thread to terminate\n");
	        	dialer.join();
		        Debug.logMessage("Dialer Thread terminated\n");
		        dialer = null;
			  } 
	        catch(Exception objE) 
	          {
	        	Debug.logMessage(objE.toString());
	          }
	      }


        Debug.closeLogger();
      }

    public void notifyExitStatus(int retVal)
      {
        if((exitStatus == 1) || (stopped == 1))
          return;

        exitStatus = 1;

        status.setForeground(Color.red);

        if(retVal == 100)
          status.setStatusText("Connection Timed out no link activity.");
        else if(retVal > 10)
          status.setStatusText("Connection closed. Code " + retVal);
        else if(retVal == -2)
	      status.setStatusText("Unable to establish connection : invalid address");
        else if(retVal == -3)
          {
			dialer = null;
			mgr = null;
			status.setStatusText("Could not add default route");
          }
        else if(strConnectionStatus == null)  //this implies no connection is available at present
          status.setStatusText("Unable to establish connection. Code " + retVal);
        else
          status.setStatusText("Connection closed");

        jbConnect.setEnabled(true);

        jbDisconnect.setEnabled(false);
      }

    public void setInfoStatus(String value)
      {
        status.setForeground(Color.black);
        status.setStatusText(value);
        if( value != null && value.startsWith("Connected to:") )
        {
          jbDisconnect.setEnabled(true);
          strConnectionStatus = value;
        }
      }

    public void setWarnStatus(String value)
      {
        status.setForeground(Color.yellow);
        status.setStatusText(value);
      }

    public void stop()
      {
	  terminateThreads();
      }

    public void destroy()
      {
	  terminateThreads();
      }
  }


package com.menlo;

import javax.swing.*;
import java.util.*;

import java.awt.*;
import java.awt.event.*;
import javax.swing.BorderFactory; 
import javax.swing.border.Border;
import javax.swing.border.EtchedBorder;
import javax.swing.JLabel;
import javax.swing.JPanel; 

import javax.swing.*;
import netscape.javascript.*;

class CustomLabel extends JLabel {

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

public class MenloSSLApplet extends JApplet implements ActionListener{
    JButton connect;
    JButton disconnect;
    JLabel label;
    CustomLabel status;
    MenloDialer dialer;
    MenloMgr mgr;
    String[] args;

    int timeOut = 0;
    String[] routes;
    String[] dns;
    String[] wins;

    String paramName;
    JPanel panel;
    Container mainPanel;
    int exitStatus = 0;
    int stopped = 0;

    public void init()
      {
        int i;
        args = new String[2];

        args[0] = getParameter("host");
        if(args[0] == null)
           args[0] = getDocumentBase().getHost();

        args[1] = getParameter("session");

        routes = new String[MenloMgr.MAX_ROUTES];
        for(i=0; i< MenloMgr.MAX_ROUTES; i++)
          {
            paramName = new String("route" + i);
            if((routes[i] = getParameter(paramName)) == null)
              break;
          }

        if(getParameter("VPNDNS") != null)
          {
            dns = new String[MenloMgr.MAX_NAME_SERVER];
            for(i=0; i< MenloMgr.MAX_NAME_SERVER; i++)
              {
                paramName = new String("DNSSERVER" + (i+1));
                if((dns[i] = getParameter(paramName)) == null)
                  break;
              }
          }
        else
          dns = null;

        if(getParameter("VPNWINS") != null)
          {
            wins = new String[MenloMgr.MAX_NAME_SERVER];
            for(i=0; i< MenloMgr.MAX_NAME_SERVER; i++)
              {
                paramName = new String("WINSSERVER" + (i+1));
                if((wins[i] = getParameter(paramName)) == null)
                  break;
              }
          }
        else
          wins = null;

        if(getParameter("timeout") != null)
          {
            Integer t = new Integer(getParameter("timeout"));
            timeOut = t.intValue();
          }

        connect = new JButton("Connect");
        disconnect = new JButton("Disconnect");
        label = new JLabel("Menlo Logic Virtual Passage", JLabel.CENTER);
        status = new CustomLabel("Click Connect to start");

        status.setToolTipText(status.getLogText());
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
        panel.add(connect);
        panel.add(disconnect);

        mainPanel.add(panel, BorderLayout.CENTER);

        status.setBorder(BorderFactory.createEtchedBorder(EtchedBorder.RAISED));
        status.setForeground(Color.blue);
        mainPanel.add(status, BorderLayout.SOUTH);

        connect.addActionListener( this ); 
        disconnect.addActionListener( this );

        connect.setActionCommand( "c" );
        disconnect.setActionCommand( "d" );   

        disconnect.setEnabled(false);

        System.out.println("Server Address:" + args[0]);
        System.out.println("Session ID:" + args[1]);

        dialer = new MenloDialer(args);
        dialer.setContainer(this);
        dialer.setTimeout(timeOut);

        mgr = new MenloMgr(routes, dns, wins);
        mgr.setContainer(this);

        setVisible(true);
      }

    public void actionPerformed( ActionEvent e )
      {
        String command = e.getActionCommand();

        if (command.equals( "c" )) {
            disconnect.setEnabled(true);
            connect.setEnabled(false);
            status.setStatusText("Connecting...");
            dialer.start();
            mgr.start();
        }
        else if (command.equals( "d" )) {
            connect.setEnabled(true);
            disconnect.setEnabled(false);
            status.setStatusText("disconnecting...");
            stop();
            JSObject win = (JSObject) JSObject.getWindow(this);
            win.eval("window.close();");
        }
      }

    public void stop()
      {
        if(stopped == 1)
          return;

        stopped = 1;

        if(mgr != null)
          mgr.stopMgr();

        if(dialer != null)
          dialer.stopConnection();

        try {
            mgr.join();
        } catch(Exception e) {};

        try {
            dialer.join();
        }catch(Exception e) {};
        
        dialer = null; 
        mgr = null;
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
        else
          status.setStatusText("Unable to establish connection. Code " + retVal);

        panel.remove(connect);
        connect.setVisible(false);

        disconnect.setText("Close");

        mainPanel.paint(getGraphics());
      }

    public void setInfoStatus(String value)
      {
        status.setForeground(Color.green);
        status.setStatusText(value);
      }

    public void setWarnStatus(String value)
      {
        status.setForeground(Color.yellow);
        status.setStatusText(value);
      }
}

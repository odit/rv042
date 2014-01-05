package com.cavium;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.StringTokenizer;

public class CaviumMgr extends Thread 
{
    private native int getIfStatus ();
    private native int waitforPPPD();
    private native int getIPv6Flag();
    private native String getInterfaceName(String strIP4Addr); 
    private native boolean 
    isServerRouteClash(String strClientLowerIP, String strClientHigherIP,String strHost, String strNetworkMask);
    private native boolean isStaticRouteClash(String strRoute, String strMask, String strHost);
    private native String getBestRoute(String strServerIP);

    CaviumSSLApplet applet = null;
    String strHost = null;
    String strOS = null;

    String strInterface = null; //global ipv6 address of the local-ppp
    boolean isIPv6Supported = false;
    private final String MAX_IPV6_PREFIX_LENGTH = "128";
    
    private boolean isDefaultRouteAdded = false;
    private boolean canEstablishPPP = true;
    private boolean isFullTunnelEnabled = false;
    private String strBestRouteToServer = null; 
    
    static 
      {
        System.load("/vpclient/libdialer.so");
      }

    public CaviumMgr(String strHostIP, String strOperatingSystem, boolean isFullTunnelEnabled)
      {
        super("CaviumMgr");
        strOS = strOperatingSystem;
        strHost = strHostIP;
        this.isFullTunnelEnabled = isFullTunnelEnabled;
      }

    public void setContainer(CaviumSSLApplet applet)
      {
        this.applet = applet;
      }

	private void logParameters(String []command)
	{
		StringBuffer sbCommandParameters = new StringBuffer();
		for(int i=0; i<command.length; i++)
		{
			sbCommandParameters.append(command[i]);
			sbCommandParameters.append(" ");
		}
		
		Debug.logMessage("About to execute : "+sbCommandParameters.toString());
	}

	private String readStream(Process child, boolean isErrorStream)
	{
      	StringBuffer buffer = new StringBuffer();
       	String temp = null;
	            	
       	try
       	{
	       	BufferedReader objBIS = 
	       	new BufferedReader(
	       		new InputStreamReader 
	       			( (isErrorStream) ? child.getErrorStream() : child.getInputStream()) );
      
	       	while( (temp = objBIS.readLine()) != null)
	       	{
	       		if(temp.length() > 0)
	       		{
		       		buffer.append(temp);
		       		buffer.append("\n");
		       	}
	       		temp = null;
	       	}
	       	objBIS.close();
       	}
       	catch(Exception objE)
       	{
	       	Debug.logMessage(objE.toString());
       	}
       	return buffer.toString();
	}// ~readStream()

    /* 
     * This method  invokes a system executable, with the specified 
     * arguments. This method space delimits the executable name and its
     * arguments.It waits for the executable to finish and returns the value
     * returned by the executable.
     */
    private int execSystem(String []strCommand)
      {
        int retVal = -1;
        try
          {
            Runtime objRuntime = Runtime.getRuntime();
            logParameters(strCommand);
            Process child =  objRuntime.exec(strCommand);
            Debug.logMessage(readStream(child,false));
            Debug.logMessage("Waiting for "+strCommand[0]+" to exit ...");
            child.waitFor();
            retVal = child.exitValue();
            Debug.logMessage(strCommand[0]+" exited with "+retVal);

            if(retVal != 0)
	           	Debug.logMessage(readStream(child,true));
          }
        catch(Exception objE)
          {
            Debug.logMessage(objE.toString());
          }
        return retVal;
      }

	private String[] makeCommand(String strCmdString)
	{
		StringTokenizer stObj = new StringTokenizer(strCmdString," ",false);
		String strRetArray[] = new String[stObj.countTokens()];
		int index = 0;
		while(stObj.hasMoreTokens())
		{
			strRetArray[index++] = stObj.nextToken();
		}
		return strRetArray;
	}

	private boolean isLinux()
	{
		return (strOS!=null && strOS.toLowerCase().indexOf("linux")!=-1);
	}
	
	private boolean isMac()
	{
		return (strOS != null && strOS.toLowerCase().indexOf("mac")!=-1);
	}

	private String getPPPInterfaceName()
	  {
		String strIFName = null;
		String strGateway = applet.getParameter("gateway");

	        Debug.logMessage("Gateway : "+strGateway);

		if(strGateway != null && strGateway.trim().length() > 0)
		  {
		  	strIFName = getInterfaceName(strGateway);
		  }

		 return strIFName;

	  }// ~getPPPInterfaceName()

    /* 
     * This method reads the static route entries, both IPv4 & IPv6, passed 
     * to it from the cgi and makes as many JNI calls as the no of route 
     * entries to add or delete them.Addition or Deletion of routes happens 
     * based on isAdd being true or false respectively.
     */
    private void manipulateRoutes(String strIfName, boolean isAdd, String strRouteFamily)
      {
      	String strRouteCount = strRouteFamily+"_count";

        if(applet.getParameter(strRouteCount) != null &&
           applet.getParameter(strRouteCount).trim().length() > 0)
          {
            int no_of_routes = Integer.parseInt(applet.getParameter(strRouteCount));
            
            Debug.logMessage("No of static route entries : "+no_of_routes);

            if(no_of_routes > 0 && strIfName != null && strIfName.trim().length() > 0)
              {
                String strRoute;
                String strOperation = (isAdd) ? "add" : "delete";
                applet.setInfoStatus((isAdd ? "Adding" : "Deleting")+" routes..");
                for(int i=0; i<no_of_routes; i++)
                  {
                    strRoute = applet.getParameter(strRouteFamily+i);
                    if(strRoute != null)
                      {
                        execSystem(makeCommand("/vpclient/routewrap"+
                        						" "+
                        						strOperation+
                        						" "+
                        						strRoute+
                        						" "+
                        						"-interface"+
                        						" "+
                        						strIfName));
                        strRoute = null;	//tell garbage collector to reclaim the 
                      }
                  }// ~for
              }// ~if(no_of_routes > 0)			
          }// ~if(applet.getParameter("route_count") != null)
      }// ~manipulateRoutes()

   /*
    * This method adds IPv6 host and network entry to the peer ipv6 address
    */
   private void manipulateHostAndNetworkRoutes(String strIfName, boolean isAdd)
     {
  	String strPeerIPv6Addr = applet.getParameter("peer_ipv6_address");
 	
  	if(strPeerIPv6Addr != null && strPeerIPv6Addr.trim().length() > 0)
  	  {
  	  	/*
  	  	 * Add IPv6 host route
  	  	 */
		String strCommand[] = 
  	  	{
  	  		"/vpclient/routewrap",
  	  		isAdd ? "add" : "delete",
  	  		"-inet6",
  	  		strPeerIPv6Addr,
  	  		"-prefixlen",
  	  		MAX_IPV6_PREFIX_LENGTH,
  	  		"-interface",
  	  		strIfName
  	  	};
  	  	
  	  	execSystem(strCommand);
  	  	
  	  	/*
  	  	 * Add IPv6 network route
  	  	 */
  	  	strCommand[5] = applet.getParameter("ipv6_prefix");
  	  	
  	  	if(strCommand[5] != null && strCommand[5].trim().length() > 0)
  	  	  {
  	  	  	execSystem(strCommand);
  	  	  }
  	  }
     }

   private void addIPv6Routes(String strIfName)
     {
     	Debug.logMessage("About to add IPv6 host & network routes\n");
        manipulateHostAndNetworkRoutes(strIfName,true);
     	Debug.logMessage("Finished adding IPv6 host & network routes\n");

     	Debug.logMessage("About to add IPv6 static routes\n");
        manipulateRoutes(strIfName,true, "routev6");
     	Debug.logMessage("Finsihed adding IPv6 static routes\n");
     }

   private void deleteIPv6Routes(String strIfName)
     {
     	Debug.logMessage("About to delete IPv6 host & network routes\n");
      	manipulateHostAndNetworkRoutes(strIfName,false);
     	Debug.logMessage("Finished deleting IPv6 host & network routes\n");

     	Debug.logMessage("About to delete IPv6 static routes\n");
        manipulateRoutes(strIfName,false, "routev6");
     	Debug.logMessage("Finsihed deleting IPv6 static routes\n");
     }

   private void addIPv4Routes(String strIfName)
     {
     	Debug.logMessage("About to add IPv4 static routes\n");
	manipulateRoutes(strIfName,true, "route");
     	Debug.logMessage("Finished adding IPv4 static routes\n");
     }

   private void deleteIPv4Routes(String strIfName)
     {
     	Debug.logMessage("About to delete IPv4 static routes\n");
        manipulateRoutes(strIfName,false, "route");
     	Debug.logMessage("Finished deleting IPv4 static routes\n");
     }

   /*
    * This method attempts to add/delete a Global-IPv6 address to 
    * the interface whose name is received as an argument, along
    * with the operation to be performed i.e add/delete.Returns 
    * true if operation succeeded otherwise return false
    */
   
   private boolean modifyGlobalAddress(String strIfName,boolean isAdd)
     {
 	boolean blnReturn = false;
  	
  	String strIPv6Address = applet.getParameter("local_ipv6_address");

  	if(strIPv6Address != null && strIPv6Address.trim().length() > 0)
  	  {
	 	String []strCommand = 
	 	{
	 		"/vpclient/ifconf",
	 		strIfName,
	 		isAdd ? "add" : "delete",
	 		strIPv6Address
	 	};
	
	 	if(execSystem(strCommand) == 0)
	 	  {
	 	  	blnReturn = true;
	 	  }
	  }// ~if(strIPv6Address != null && strIPv6Address.trim().length() > 0)

	return blnReturn;	  	  

     }// ~modifyGlobalAddress(...)

   private boolean addGlobalAddress(String strIfName)
     {
 	return modifyGlobalAddress(strIfName,true);
     }

   private boolean deleteGlobalAddress(String strIfName)
     {
 	return modifyGlobalAddress(strIfName,false);
     }

   private void getIPv6Support()
     {
  	isIPv6Supported = (getIPv6Flag() == 1) ? true : false;
     }
 
   /*
    * This method adds/deletes IPv4 network route entry
    */
   private void manipulateNetworkRoute
   				(String strIfName, boolean isAdd, String strRouteType)
     {
     	String strNetworkAddress = null;
     	String strNetworkMask = null; 
     	
     	if("network".equals(strRouteType))
     	  {
     	  	strNetworkAddress = this.applet.getParameter("gateway");
	     	strNetworkMask = this.applet.getParameter("network_mask");
	      }
     	  
     	if(strNetworkAddress == null || strNetworkAddress.trim().length() == 0 || 
     	   strNetworkMask == null || strNetworkMask.trim().length() == 0)
     	  {
     	  	return;
     	  }
     	
        String strCommand[] = 
        {
        	"/vpclient/routewrap",
        	isAdd ? "add" : "delete",
        	"-net",
        	strNetworkAddress,
        	"-netmask",
        	strNetworkMask,
        	"-interface",
        	strIfName,
        };

        execSystem(strCommand);
     }
     
   private void deleteNetworkRoute(String strIfName)
     {
     	Debug.logMessage("About to delete IPv4 network route\n");
     	manipulateNetworkRoute(strIfName, false, "network");
     	Debug.logMessage("Finished deleting IPv4 network route\n");
     }
     
   private void addNetworkRoute(String strIfName)
     {
     	Debug.logMessage("About to add IPv4 network route\n");
     	manipulateNetworkRoute(strIfName, true, "network");
     	Debug.logMessage("Finished adding IPv4 network route\n");
     }
     
   /*
    * This method gets the best route to the server, through a
    * JNI call, and adds a host entry through the best route.
    */
   private boolean manipulateDefaultRoute(boolean isAdd)
     {
     	int ret_val = -1;

     	/* getBestRoute - JNI call */
     	if(strBestRouteToServer == null)
     		strBestRouteToServer = new String(getBestRoute(strHost));

     	if(strBestRouteToServer.trim().length() > 0)
     	  {
	     	ret_val = 
    	 	execSystem(makeCommand
	    	 	("/vpclient/routewrap "+(isAdd ? "add " : "delete ")+strBestRouteToServer.trim()));
    	  }
    	else
    	  {
    	  	Debug.logMessage("Could not determine best route"+strBestRouteToServer+"\n");
    	  }
     		
     	return (ret_val == 0) ? true : false;
     }
   
   /*
    * This method finds if a default route was added
    * earlier and if so, goes on to delete that route.
    */
   private void deleteDefaultRoute()
     {
   		Debug.logMessage("About to delete default route\n");
	  	manipulateDefaultRoute(false);
   		Debug.logMessage("Finished deleting default route\n");
     }
   
   /*
    * This method finds if a default route is needed
    * and if so, goes on to add a default route.
    */
   private void addDefaultRoute()
     {
     	Debug.logMessage("About to add default route\n");
     	isDefaultRouteAdded = manipulateDefaultRoute(true);
     	Debug.logMessage("Finished adding default route\n");
     }

   /*
    * This method returns true if one of the following conditions is satisfied...
    * 	1.if the server address is within the VP client address range
    * 	2.if the n/w part of the server address matches with the n/w part of 
    *	  one of the static route entry's address.
    * Server route clash or Static route clash are determined through JNI calls.
    */
   private boolean isDefaultRouteNeeded()
     {
     	String strClientLowerIP = null;
     	String strClientHigherIP = null;
		String strNetworkMask = null;

		String strRoute = null;
		StringTokenizer stTokens = null;
		
     	int iterator, no_of_routes = 0;
     	     	
     	if(isIPv6Supported)
		  {
/*
			strClientLowerIP = this.applet.getParameter("client_lower_ipv6");
			strClientHigherIP = this.applet.getParameter("client_higher_ipv6");
			strNetworkMask = this.applet.getParameter("ipv6_prefix");
*/
			Debug.logMessage("Default route addition not supported for IPv6\n");			
			return false;
		  }
		else
		  {
			strClientLowerIP = this.applet.getParameter("client_lower_ip");
			strClientHigherIP = this.applet.getParameter("client_higher_ip");
			strNetworkMask = this.applet.getParameter("network_mask");
		  }
		  
		if( strClientLowerIP != null && strClientLowerIP.trim().length() > 0 &&
     	    strClientHigherIP != null && strClientHigherIP.trim().length() > 0 &&
     	    strNetworkMask != null && strNetworkMask.trim().length() > 0)
     	  {
			/* isServerRouteClash() - JNI call */
			if(isServerRouteClash(strClientLowerIP, strClientHigherIP, strHost, strNetworkMask))
			  {
				Debug.logMessage("Server Route Clash - Default Route is needed");
				return true;
			  }
     	  }
		else
		  {
     	  	  Debug.logMessage("Invalid client address range");
		  }     	  

		no_of_routes = Integer.parseInt(this.applet.getParameter("route_count"));
		for(iterator=0; iterator<no_of_routes; iterator++)
		  {
		  	strRoute = this.applet.getParameter("route"+iterator);
			stTokens = new StringTokenizer(strRoute);
			/* isStaticRouteClash() - JNI call */
			if(isStaticRouteClash(stTokens.nextToken("-net").trim(), 
								  stTokens.nextToken("-netmask").trim(), 
								  strHost))
			  {
				Debug.logMessage("Static Route Clash - Default Route is needed");
				return true;
			  }
		  }
		
		Debug.logMessage("No Default Route is needed");

		return false;
     }
     
   /*
    * This method adds/deletes DNS entry by executing a binary
    */
   private void manipulateDNS(String strInterface, boolean isAdd)
     {
     	String strDomain = "";
     	String strPrimaryDNS = "";
     	String strSecondaryDNS = "";

		boolean canInvoke = false;
		
     	if( !isAdd )
     	  {
     	  	if(isMac())
     	  	  {
	     	  	String strCmd[] = 
	     	  	{
	     	  		"/vpclient/dnscommand",
	     	  		"/vpclient/dnscommand.sh",
	     	  		"unset",
	     	  		strInterface
	     	  	};
	     		execSystem(strCmd);
     	  	  }
     	  	else if(isLinux())
     	  	  {
	     	  	String strCmd[] = 
	     	  	{
	     	  		"/vpclient/dnscommand",
	     	  		"unset"
	     	  	};
	     		execSystem(strCmd);
     	  	  }
     	  	return;
     	  }
     	  
     	if(this.applet.getParameter("DOMAIN") != null)
     	  {
     	  	strDomain = this.applet.getParameter("DOMAIN").trim();
     	  	canInvoke = (strDomain.length() > 0);
     	  }

     	if(this.applet.getParameter("DNSSERVER1") != null)
     	  {
     		strPrimaryDNS = this.applet.getParameter("DNSSERVER1").trim();

     	  	if(strPrimaryDNS.length() > 0 && this.applet.getParameter("DNSSERVER2") != null)
     	  	  {
     	  	  	strSecondaryDNS = this.applet.getParameter("DNSSERVER2").trim();
     	  	  }
     	  
     	   	canInvoke = true;
		  }
     	
     	if(canInvoke)	//do not invoke if no parameter is available
     	  {
     	  	if(isMac())
     	  	  {
	     	  	String strCmd[] =
	     		{
	     	  		"/vpclient/dnscommand",
	     	  		"/vpclient/dnscommand.sh",
	     			"set",
	     			strDomain,
	     			strPrimaryDNS,
	     			strSecondaryDNS,
	     			strInterface
	     		}; 
	     		execSystem(strCmd);
     	  	  }
     	  	else if(isLinux())
     	  	  {
	     	  	String strCmd[] =
	     		{
	     	  		"/vpclient/dnscommand",
	     			"set",
	     			strDomain,
	     			strPrimaryDNS,
	     			strSecondaryDNS
	     		}; 
	     		execSystem(strCmd);
     	  	  }
     	  }
     }// ~manipulateDNS

   private void addDNS(String strInterface)
     {
		Debug.logMessage("About to add DNS");
   		manipulateDNS(strInterface, true); 
   		Debug.logMessage("Finished adding DNS");	
     }

   private void deleteDNS(String strInterface)
     {
		Debug.logMessage("About to delete DNS");
   		manipulateDNS(strInterface, false); 
   		Debug.logMessage("Finished deleting DNS");	
     }

   boolean canPPPConnect()
     {
	   getIPv6Support();
   	   if(!isIPv6Supported && (isFullTunnelEnabled || isDefaultRouteNeeded()))
   	     {
   	     	addDefaultRoute();
			
			/* 
			 * Do not proceed if default route was
			 * needed but could not be added
			 */
			canEstablishPPP = isDefaultRouteAdded;
   	     }
     	return canEstablishPPP;
     }
   /* 
    * This method waits for PPP connection to be established by making a
    * JNI call and then goes on to add Network and Static route entries.
    * Once PPP is up, a file is created and once it happens  the JNI 
    * function returns.Till that time it waits for the same to be created.
    */
   public void run()
     {
       if(waitforPPPD() < 0)
         {
           Debug.logMessage("Mgr: PPP Interface failure");
           applet.notifyExitStatus(-1);
           return;
         }

       applet.setInfoStatus("Connection established.");
       try 
         {
           Thread.sleep(1);
         }
       catch(Exception objE) 
         {
       	   Debug.logMessage(objE.toString());
         }

       strInterface = getPPPInterfaceName();
        
       /*
        * Addition of both IPv4 and IPv6 routes happens using the
        * interface name, hence if PPP's interface name can't be  
        * obtained, no point in attempting to add routes over that.
        */
        if(strInterface != null && strInterface.trim().length() > 0)
          {
       	    Debug.logMessage("PPP Interface Name : "+strInterface);

	   	    if(!isFullTunnelEnabled || isIPv6Supported)
   		      {
				addNetworkRoute(strInterface);
				addIPv4Routes(strInterface);
   			  }

	    /*
	     * If a Gobal-IPv6 address could not be assigned to the PPP ,
	     * then even if routes are added, the packets will still be
	     * discarded because of the limitations of the link-local addr.
	     * Hence add IPv6 routes only if a Global-IPv6 addr is assigned.
	     */
	    if(isIPv6Supported && addGlobalAddress(strInterface))
	      {
	        addIPv6Routes(strInterface);
	      }
			addDNS(strInterface);
          }// ~if(strInterface != null && strInterface.trim().length() > 0)

        applet.setInfoStatus("Connected to:  "+strHost);
     }// ~run()

    /* 
     * This method attempts to delete the route entries recevied from cgi ,
     * whether it got added successfully or not.The function will return
     * gracefully if it couldn't delete a route entry.Also failure in deletion 
     * of a route entry will not cause deletion of other route entries to be affected.
     */
   public void stopMgr()
     {
        if(strInterface != null && strInterface.trim().length() > 0)
      	  {
      	  	if(isDefaultRouteAdded)
      	  	  {
      	  	  	deleteDefaultRoute();
      	  	  }
      	  	if(!isFullTunnelEnabled || isIPv6Supported)
      	  	  {
	      	  	deleteNetworkRoute(strInterface);
	      	  	deleteIPv4Routes(strInterface);
      	  	  }
      	  	if(isIPv6Supported)
      	  	  {
      	  	  	deleteGlobalAddress(strInterface);
      	  	  	deleteIPv6Routes(strInterface);
      	  	  }
	      	 deleteDNS(strInterface);
      	  }
     }

   public int getStatus()
     {
   	return getIfStatus();
     }
}

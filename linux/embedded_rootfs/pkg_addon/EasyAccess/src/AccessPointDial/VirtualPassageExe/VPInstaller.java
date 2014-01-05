//Required for Installer
import java.util.Date;

import java.net.URL;
import java.net.URLConnection;

import java.io.File;
import java.io.InputStream;
import java.io.BufferedReader;
import java.io.FileOutputStream;
	import java.io.InputStreamReader;

import java.text.DateFormat;

import java.util.ArrayList;
import java.util.StringTokenizer;

import java.util.jar.Manifest;

import javax.swing.JApplet;
import javax.swing.JOptionPane;

//Required for VPExtracter
import java.io.OutputStream;
import java.io.BufferedOutputStream;

import java.util.jar.JarFile;
import java.util.Enumeration;
import java.util.jar.JarEntry;

/* 
 * The custom class for redirecting log messages
 */
class VPDebug
{
	static void logMessage(String strMessage)
	{
		System.err.println(strMessage);
	}
}

/* 
 * This class extracts a list of files specified through strFileNames,
 * from the zipped file passed through strFileName.The files are extracted
 * to the same location as the zipped file.This class's abstraction is such
 * that it eliminates the need to have jar utility separately installed.
 */
class VPExtracter
{
    private String strJarFileName = null;
    private String strCompressedFile = null;
    private String strDestination = null;

	private final int RETRY_WAIT = 8000;	//milliseconds
	private int RETRY_ATTEMPT = 0;

    private boolean unzip(JarFile objJarFile, JarEntry objJarEntry)
      {
        byte[] buffer = new byte[1024];
		
        int len;
        boolean blnRet_val = false;
        InputStream objIS = null;
        OutputStream objOS = null;
        try
          {
            objIS = objJarFile.getInputStream(objJarEntry);
            objOS = new BufferedOutputStream
                    ( new FileOutputStream(this.strDestination+objJarEntry.getName()) );

            while((len = objIS.read(buffer)) >= 0)
			{
              objOS.write(buffer, 0, len);
			}

            blnRet_val = true;

            objIS.close();
            objOS.close();
          }// ~try
        catch(Exception objE)
          {
          	VPDebug.logMessage(objE.toString());
          }

        return blnRet_val;
      }// ~unzip

	boolean isManifestFile(JarEntry objJarEntry)
	{
		String strFileName = objJarEntry.getName();
		boolean blnRetval = (strFileName.equalsIgnoreCase("MANIFEST.MF")  || 	
							 strFileName.equalsIgnoreCase("META-INF/MANIFEST.MF"));
		return blnRetval;
	}
	
    boolean extractFiles(String strJarFile, String strDestn)
      {
        JarFile objJarFile= null;
        JarEntry objJarEntry = null;
        this.strJarFileName = strJarFile;	//File to be extracted
        this.strDestination = strDestn;		//Destination where to be extracted
        boolean blnRetVal = true;
        InputStream objIS = null;
        OutputStream objOS = null;

        try 
          {
            objJarFile= new JarFile(strJarFileName);	// Open the JAR file

            //Iterate through the JAR file entries
            for (Enumeration entries = objJarFile.entries(); entries.hasMoreElements();) 
              {
                objJarEntry = (JarEntry)entries.nextElement();

				if(isManifestFile(objJarEntry))
					continue;
               	do
               	{
               		VPDebug.logMessage("Decompressing "+objJarEntry.getName());
	               	if(!unzip(objJarFile, objJarEntry))
		              {
						RETRY_ATTEMPT++;

						if(RETRY_ATTEMPT == 3)
							break;

						VPDebug.logMessage("Retry attempt "+RETRY_ATTEMPT);

						try
						{
							Thread.sleep(RETRY_WAIT);
						}
						catch(Exception objE)
						{
							VPDebug.logMessage(objE.toString());
						}
   		              }
   		            else
   		            	break;
    	       	}while(true);
               	
               	if(RETRY_ATTEMPT == 3)
               	  {
					blnRetVal = false;
   	               	break;
   	              }
   
              }// ~for(...)

           objJarFile.close();

          }// ~try
        catch (Exception objE) 
          {
			blnRetVal = false;
            VPDebug.logMessage(objE.toString());
          }
	
        return 	blnRetVal;       

      }// ~extractFiles()

}// ~VPExtracter


public class VPInstaller extends JApplet 
{
    static final long serialVersionUID = 0;
    private String strURL = null;
    private String strHostIP = null;

	private	final String LAUNCH_VP = "1";
	private	final String DISCONNECT_VP = "2";
 
	private	final String FILE_ARG_DELIMITER = ";";

	private final String strInstallerFiles[] = 
	{
		"SSLDrv.sys",
		"SSLDrv.txt",
		"UninstallVTPassage.exe"
	};
	
    private final String strDllFile = "XTunnel.dll";
    private final String strVirtualPassage = "VirtualPassageExe.exe";
    private String strFileSeparator = System.getProperty("file.separator");
	private final String strUserHome = System.getProperty("user.home");
    
    private String 
		    strInstallDir = strUserHome + strFileSeparator + "VirtualPassage" + strFileSeparator;
    
    private String strFileName = null;
    
    private File objFile = null;	
    private boolean blnIsFileCorrupted = false, blnIsUpdatePending = false;
        
    public void init()
      {
		VPDebug.logMessage("OS : "+System.getProperty("os.name"));
		VPDebug.logMessage("Arch : "+System.getProperty("os.arch"));
		VPDebug.logMessage("Home : "+strUserHome);

		
        strHostIP = getDocumentBase().getHost();
		VPDebug.logMessage("getDocumentBase().getHost() : "+strHostIP);

		if(strHostIP == null || 0 == strHostIP.trim().length())
		{
			strHostIP = getParameter("host");
			VPDebug.logMessage("getParameter : "+strHostIP);
		}			

		strURL = "https://" + strHostIP + "/";
        VPDebug.logMessage("Source URL : "+strURL);

		strFileName = getParameter("dialer_jar");
		if(strFileName == null || 0 == strFileName.trim().length())
		  {
			strFileName = "WindowsVPDialer.jar";
		  }

        objFile = new File(strInstallDir);
      }

	
	private String getRoutes()
	  {
		StringBuffer sbRoutes = new StringBuffer();
		


		return sbRoutes.toString().trim();
	  }// ~getRoutes


	private String getToken(String strArg, int token_number)
	  {
		String strReturn = "";
		StringTokenizer stObj = new StringTokenizer(strArg," ",false);

		if(token_number > 0 && token_number <= stObj.countTokens())
		  {
			while(token_number > 1)
			{
				stObj.nextToken();
				token_number--;
			}
			
			strReturn = stObj.nextToken();
		  }

		return strReturn;
	  }

    private void getRoutes(ArrayList alCmdLineArgs, String strRouteFamily)
      {
      	String strRouteCount = strRouteFamily+"_count";
		String strTemp;

		final int ADDR = 1;
		final int MASK = 2;

		int route_count;

        strTemp = getParameter(strRouteCount);
		route_count = (strTemp != null) ? Integer.parseInt(strTemp) : 0;

		strTemp = null;	//tell garbage collector to take it away

		VPDebug.logMessage("No of "+strRouteFamily+" entries : "+route_count);

		alCmdLineArgs.add(new String(""+route_count));
		
		for(int i=0; i<route_count; i++)
		  {
			strTemp = getParameter(strRouteFamily+i);
			if( strTemp != null)
			  {
				alCmdLineArgs.add(getToken(strTemp, ADDR));
				alCmdLineArgs.add(getToken(strTemp, MASK));
				strTemp = null; // tell garbage collector that this string memory can be reclaimed
			  }
		  }
	  }// ~getRoutes()

	private void addIPv4Routes(ArrayList alCmdLineArgs)
	  {
	  	getRoutes(alCmdLineArgs, "route");
	  }

	private void addIPv6Routes(ArrayList alCmdLineArgs)
	  {
	  	getRoutes(alCmdLineArgs, "routev6");
	  }

	private void addGlobalAddresses(ArrayList alCmdLineArgs)
	  {
	  	String strTemp = null;

	  	strTemp = getParameter("local_ipv6_address");
	  	
	  	if(strTemp != null)
	  	  {
	  	  	alCmdLineArgs.add(strTemp);
	  	  	strTemp = null;
	  	  }

	  	strTemp = getParameter("peer_ipv6_address");
	  	
	  	if(strTemp != null)
	  	  {
	  	  	alCmdLineArgs.add(strTemp);
  	  		strTemp = null;
	  	  }

	  	strTemp = getParameter("ipv6_prefix");
	  	
	  	if(strTemp != null)
	  	  {
	  	  	alCmdLineArgs.add(strTemp);
  	  		strTemp = null;
	  	  }

	  	strTemp = getParameter("client_lower_ipv6");
	  	
	  	if(strTemp != null)
	  	  {
	  	  	alCmdLineArgs.add(strTemp);
  	  		strTemp = null;
	  	  }

	  	strTemp = getParameter("client_higher_ipv6");
	  	
	  	if(strTemp != null)
	  	  {
	  	  	alCmdLineArgs.add(strTemp);
  	  		strTemp = null;
	  	  }

	  	strTemp = getParameter("force_ipv6");
	  	
	  	if(strTemp != null)
	  	  {
	  	  	alCmdLineArgs.add(strTemp);
  	  		strTemp = null;
	  	  }
	  }

    private void invokeVP()
      {
		final int ADDR = 1;
		final int MASK = 2;
	
		ArrayList alCmdLineArgs = new ArrayList();

		alCmdLineArgs.add(strInstallDir+strVirtualPassage);		//Path+Executable
		alCmdLineArgs.add(FILE_ARG_DELIMITER);
		alCmdLineArgs.add(LAUNCH_VP);							//Launch or Disconnect VP ?
		alCmdLineArgs.add(strHostIP);							//IPAddress
		alCmdLineArgs.add(getParameter("session"));				//SessionID
		alCmdLineArgs.add(getParameter("client_lower_ip"));		//Client IP Addr range - lower limit
		alCmdLineArgs.add(getParameter("client_higher_ip"));	//Client IP Addr range - upper limit
		alCmdLineArgs.add("1");									//SSLTunnel ?
		alCmdLineArgs.add("443");								//ServerPort
		alCmdLineArgs.add("0");									//Uninstall Clicked ?
		alCmdLineArgs.add("0");									//Cert Root Validate ?
		alCmdLineArgs.add(getParameter("dns_suffix"));			//DNS suffix
		alCmdLineArgs.add(getParameter("network_mask"));		//Network mask
		alCmdLineArgs.add(getParameter("enable_full_tunnel"));	//Full tunnel mode ?
		/*
		 * Populate the IPv4 addresses into 
		 * the command line arguments list.
		 */
		addIPv4Routes(alCmdLineArgs);

		/*
		 * Populate ...
		 *
		 * 1) the local PPP's Global-IPv6 address
		 * 2) the remote PPP's Global-IPv6 address
		 * 3) the client_lower_ipv6 & client_higher_ipv6 prefix
		 * 4) the client_lower_ipv6 address
		 * 5) the client_higher_ipv6 address
		 * 6) the force_ipv6 flag
		 * 
		 * into the command line arguments list.
		 */
		addGlobalAddresses(alCmdLineArgs);

		/*
		 * Populate the IPv6 addresses into 
		 * the command line arguments list.
		 */
		addIPv6Routes(alCmdLineArgs);

      	//copyFilesToSystemRoot();
   	   	//registerActiveX();

		String []command = new String[alCmdLineArgs.size()];

		command = (String [])alCmdLineArgs.toArray(command);

		if(execSystem(command,false) == 0) 
		{
			VPDebug.logMessage(command[0]+" successfully invoked");
		}
		else
		{
			VPDebug.logMessage(command[0]+" execution failed...!");
			reportVirtualPassageExeError();
		}

      }// ~invokeVP()

	private void reportInstallError()
	  {
	    //Error occured while creating the directory : Insufficient privilege for installation
        JOptionPane.showMessageDialog(null,"Error : Check if..\n\n\t1. you have administrative privileges on your system \n\t2. there is sufficient disk space","Installation Failed",JOptionPane.ERROR_MESSAGE);
	  }
	
	private void reportVirtualPassageExeError()
	  {
		//Error occured while executing system with command line arguments
		JOptionPane.showMessageDialog(null,"Error : Virtual Passage Execution Failed","Installation Failed",JOptionPane.ERROR_MESSAGE);
	  }
	private void reportDownloadError()
	  {
		//Error occured while downloading the file : Connection error
        JOptionPane.showMessageDialog(null, "An error occured while downloading VPClient.\nUninstall and then try installing again.", "Error downloading files", JOptionPane.ERROR_MESSAGE);
	  }

	private void updateAvailable()
	  {
		//Intimate the user of the availability of a newer version
	    JOptionPane.showMessageDialog(null, "An updated version of VPClient is available.To update, just click on the\n \"Launch\" icon after logging in with administrative privilege.","Update Available...!", JOptionPane.INFORMATION_MESSAGE);

	  }

	private void reportFileCorrupted()
	  {
        JOptionPane.showMessageDialog(null, "VPClient has been found to be corrupted and needs to be reinstalled", "Error extracting installation files", JOptionPane.ERROR_MESSAGE);
	  }
	
	private boolean unzipSuccess()
	  {
		  return 
		  ( new VPExtracter().extractFiles(strInstallDir+strFileName, strInstallDir) );
	  }
	
    public void start()
      {
        switch( copyFiles() )
          {
                case -1:
                		reportInstallError();
                        break;

                        
                case -2:
                		reportDownloadError();
                        break;
                        
                case -3:			
						updateAvailable();
						VPInitialize();
						break;

	            case 0:  /* File was already existing and none had to be downloaded */
				default: /* File was successfully downloaded */
						VPInitialize();
						break;
                		
           }// ~switch
      }

	private void VPInitialize()
	{
        if(unzipSuccess())
        {
			invokeVP();
        }  	
        else
        {
			reportFileCorrupted();
        }
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
	       	VPDebug.logMessage(objE.toString());
       	  }
       	return buffer.toString();
	  }

	private void logParameters(String []command)
	  {
		StringBuffer sbCommandParameters = new StringBuffer();
		for(int i=0; i<command.length; i++)
		  {
			sbCommandParameters.append(command[i]);
			sbCommandParameters.append(" ");
		  }
		
		VPDebug.logMessage("About to execute : "+sbCommandParameters.toString());
	  }
		
    /* 
     * This method  invokes a system executable, with the specified 
     * arguments. This method space delimits the executable name and its
     * arguments.It waits for the executable to finish and returns the value
     * returned by the executable.
     */
    private int execSystem(String []command, boolean waitForChildToExit)
      {
        StringBuffer sbOutput = new StringBuffer();
        int retVal = 0; 
        try
          {
            Runtime objRuntime = Runtime.getRuntime();
            logParameters(command);
            Process child =  objRuntime.exec(command);
            if(waitForChildToExit)
              {
	            sbOutput.append(readStream(child,false));
	            VPDebug.logMessage("Waiting for "+command[0]+" to finish...");
	            child.waitFor();
	            retVal = child.exitValue();
	            if(retVal != 0)
		          {
		        	sbOutput.append("Error : \n");
	            	sbOutput.append(readStream(child,true));
	              }
              }// ~waitForChildToExit
            VPDebug.logMessage(sbOutput.toString());
          }
        catch(Exception objE)
          {
          	retVal = -1;
            VPDebug.logMessage(objE.toString());
          }
        return retVal;
      }


    private boolean isUpdateAvailable(String srcURL, String destFile)
      {
        boolean blnIsUpdateAvailable = false;

        JarFile objClientJar = null;
        Manifest objManifest = null;
        String strVersion = null;
        try
          {
			objClientJar = new JarFile(destFile);
			
			if(objClientJar.getManifest() == null)
			  {
			  	VPDebug.logMessage("Could not find Manifest file");
			  	return blnIsUpdateAvailable;
			  }

			objManifest = objClientJar.getManifest();

			if(objManifest.getAttributes("Virtual Passage") == null)
			  {
			  	VPDebug.logMessage("Could not read Entry");
			  	return blnIsUpdateAvailable;
			  }

			strVersion = (objManifest.getAttributes("Virtual Passage")).getValue("Version");

			if( strVersion == null)
			  {
			  	VPDebug.logMessage("Could not read Version");
			  	return blnIsUpdateAvailable;
			  }
			
			blnIsUpdateAvailable = !(strVersion.equals(getParameter("jar_version")));
          }
        catch(Exception objE)
          {
            VPDebug.logMessage(objE.toString());
          }

       	VPDebug.logMessage("Server Jar Version : "+getParameter("jar_version"));
       	VPDebug.logMessage("Client Jar Version : "+strVersion);

	  	VPDebug.logMessage("Update Available : "+blnIsUpdateAvailable);

        return blnIsUpdateAvailable;
      }

    private int copyFiles() 
      {
        int no_of_files_copied = 0;

        VPDebug.logMessage(strInstallDir+" exists : "+objFile.exists());    
        if(!objFile.exists())
          {
            if(!objFile.mkdir())
              {
                strInstallDir = null;
                no_of_files_copied = -1;
              }
          }

        VPDebug.logMessage("Created Directory : "+strInstallDir);   

        if(strInstallDir != null)
          {
            if( !(new File(strInstallDir + strFileName).exists()) )
              {
                if( !objFile.canWrite() )
                  no_of_files_copied = -1;
                else
                  {
                    if( copyURL(strURL + strFileName, strInstallDir + strFileName) == 0)
                      no_of_files_copied++ ;
                    else
                      no_of_files_copied = -2;
                  }// ~else
              }
            else
              {
                if( isUpdateAvailable(strURL + strFileName, strInstallDir + strFileName) )
                  {
                    if( !objFile.canWrite() )
                      no_of_files_copied = -3 ;
                    else
                      {
                        //If download of latest version succeeds ...
                        if( copyURL(strURL+strFileName, strInstallDir+strFileName) == 0 )
                            no_of_files_copied++ ;
                        else
	                        no_of_files_copied = -2;
                      }
                  }// ~if(isUpdateAvailable....)
              }
          }// ~if(strInstallDir != null)
        return no_of_files_copied;
      }// ~copyFiles


    /* 
     * This method copes the file specified in the srcURL to destFile
     * side by side updating the user on the progress of file download.
     * This method returns 0 on success, -1 on error.
     */
    private int copyURL(String srcURL, String destFile)
      {
        int retVal = 0;
        try
          {
            URL  objURL  = new URL(srcURL);
            int file_size = 0;
            VPDebug.logMessage("Opening connection to " + srcURL + "...");
            URLConnection urlC = objURL.openConnection();

            VPDebug.logMessage("URL.openStream . . .");
            InputStream is = objURL.openStream();

            VPDebug.logMessage("Resource type : " + urlC.getContentType());
            Date date=new Date(urlC.getLastModified());
            VPDebug.logMessage("Last modified on : " + DateFormat.getDateInstance().format(date));
            file_size = urlC.getContentLength();
            VPDebug.logMessage("File size : "+file_size);
            System.out.flush();

            FileOutputStream fos=null;
            fos = new FileOutputStream( new File(destFile) );
            int bytes_read, count=0;

            VPDebug.logMessage("Copying " + srcURL +" to "+ destFile);

            while ((bytes_read=is.read()) != -1)
              {
                fos.write(bytes_read);
                count ++;
              }
            is.close();
            fos.close();

            VPDebug.logMessage(count + " byte(s) copied");
          }// ~try
        catch(Exception objE)
          { 
            retVal = -1;
            VPDebug.logMessage(objE.toString());
          }
        return retVal;
      }//~copyURL

	public void stop()
	  {
		System.err.println("VP disconnect invoked");
		ArrayList alCmdLineArgs = new ArrayList();

		alCmdLineArgs.add(strInstallDir+strVirtualPassage);		//Path+Executable
		alCmdLineArgs.add(FILE_ARG_DELIMITER);
		alCmdLineArgs.add(DISCONNECT_VP);						//Launch or Disconnect VP ?
		String []command = new String[alCmdLineArgs.size()];

		command = (String [])alCmdLineArgs.toArray(command);

		if(execSystem(command,false) == 0) 
		{
			VPDebug.logMessage(command[0]+" successfully invoked");
		}
		else
		{
			VPDebug.logMessage(command[0]+" execution failed...!");
		}
	  	
	  }

}// ~Installer

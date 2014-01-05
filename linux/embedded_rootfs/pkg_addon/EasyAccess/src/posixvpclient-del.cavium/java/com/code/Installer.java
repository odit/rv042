//Required for Installer
import java.util.Date;

import java.net.URL;
import com.code.Debug;
import java.net.URLConnection;

import java.io.File;
import java.io.InputStream;
import java.io.FileOutputStream;

import java.awt.Container;
import java.awt.BorderLayout;

import java.text.DateFormat;

import javax.swing.JApplet;
import javax.swing.JLabel;
import javax.swing.JPanel; 
import javax.swing.JOptionPane;
import javax.swing.JProgressBar;
import javax.swing.JPasswordField;

import java.io.BufferedReader;
import java.io.BufferedWriter;

import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

import java.util.StringTokenizer;
import java.util.jar.Manifest;

import netscape.javascript.JSObject;

//Required for Extracter
import java.io.OutputStream;
import java.io.BufferedOutputStream;

import java.util.jar.JarFile;
import java.util.Enumeration;
import java.util.jar.JarEntry;

/* 
 * This class extracts a list of files specified through strFileNames,
 * from the zipped file passed through strFileName.The files are extracted
 * to the same location as the zipped file.This class's abstraction is such
 * that it eliminates the need to have jar utility separately installed.
 */
class Extracter
{
    private String strJarFileName = null;
    private String strCompressedFile = null;
    private String strDestination = null;

    private boolean fileExists(String strFileName, String[] strFileNames)
      {
        boolean blnRet_val = false;
        if(strFileName != null)
          {
            for(int index=0; index<strFileNames.length; index++)
              {
                if(strFileName.equals(strFileNames[index]))
                  {
                    blnRet_val = true;
                    break;
                  }
              }// ~for(...)
          }// ~if(strFileName != null)
        return blnRet_val;
      }// ~fileExists

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
              objOS.write(buffer, 0, len);

            blnRet_val = true;

            objIS.close();
            objOS.close();
          }// ~try
        catch(Exception objE)
          {
          	Debug.logMessage(objE.toString());
          }

        return blnRet_val;
      }// ~unzip

    boolean extractFiles(String strJarFile, String[] strFileNames, String strDestn)
      {
        JarFile objJarFile= null;
        JarEntry objJarEntry = null;
        this.strJarFileName = strJarFile;	//File to be extracted
        this.strDestination = strDestn;		//Destination where to be extracted
        boolean blnReturn = true;
        InputStream objIS = null;
        OutputStream objOS = null;

        try
          {
            objJarFile= new JarFile(strJarFileName);	// Open the JAR file

            //Iterate through the JAR file entries
            for (Enumeration entries = objJarFile.entries(); entries.hasMoreElements();) 
              {
                objJarEntry = (JarEntry)entries.nextElement();
                if( fileExists(objJarEntry.getName(), strFileNames) )
                  {
                    if(unzip(objJarFile, objJarEntry))
                      {
                      	Debug.logMessage("Decompressing : "+objJarEntry.getName());
                      }
                    else
                      {
                      	Debug.logMessage("Error while decompressing "+objJarEntry.getName());
                      	blnReturn = false;
                      	break;
                      }
                  }
              }// ~for(...)

            objJarFile.close();

          }// ~try
        catch (Exception objE) 
          {
            Debug.logMessage(objE.toString());
          }

        return blnReturn;
      }// ~extractFiles()

}// ~Extracter


public class Installer extends JApplet 
{
    static final long serialVersionUID = 0;
    private String strURL = null;

    //Files needed for running VP on Linux & MAC
    private final String strRoute = "route";					//adds/deletes route
    private final String strIfConf = "ifconf";					//adds/deletes interface address
    private final String strPPPWrap = "pppwrap";
    private final String strRouteWrap = "routewrap";
    private final String strDNSCommand = "dnscommand";			//adds/deletes dns entries
    private final String strJNILibrary = "libdialer.so";
	
    //Files needed for running VP on MAC 
    private final String strDNSScript = "dnscommand.sh";
    private final String strMacConfigScript = "macconfig.sh";
	private final String strUninstallScript = "uninstaller.sh";
	
	private final String strMacInstallationFiles[] = 
    {
    	strRoute,
    	strPPPWrap,
    	strRouteWrap,
    	strJNILibrary,
    	strDNSCommand,
    	strDNSScript,
    	strUninstallScript,
	strIfConf,			//used only for IPv6, hence not mandatory to be present inside jar
    	strMacConfigScript	//important - always config script should be the last in this list
    };

	//Files needed for running VP on Linux	
    private final String strIPUP = "ip-up";						//executes after IPCP is up
    private final String strPPPD = "pppd.vp";					//customized PPP program
    private final String strIPDOWN = "ip-down";					//executes after IPCP is down
    private final String strLinuxConfigScript = "linuxconfig.sh"; //sets file permissions

    private final String strLinuxInstallationFiles[] = 
	{
		strPPPD,
		strIPUP,
		strRoute,
		strIPDOWN,
		strPPPWrap,
		strRouteWrap,
		strDNSCommand,
		strJNILibrary,
		strIfConf,				//used only for IPv6, hence not mandatory to be present inside jar
		strLinuxConfigScript	//important - always config script should be the last in this list
	};
	
	private String strExtractFiles[] = null;
	
	private String strOperatingSystem = null;
	
    private String strFileSeparator = System.getProperty("file.separator");
    private String strInstallDir = "/vpclient" + strFileSeparator;
    private String strRedirectURL = null;
    private String strFileName = null;
    
    private JProgressBar progressBar = null;
    private Container mainPanel = null;
    private JPanel panel = null;
    private JLabel oJLabel = null;

    private File objFile = null;	

    private boolean blnInstallSuccess = false;

    public void init()
      {
        mainPanel = getContentPane();

        progressBar = new JProgressBar(0, 100);
        progressBar.setValue(0);
        progressBar.setStringPainted(false);

        oJLabel = new JLabel("File Download in progress...", JLabel.CENTER);

        panel = new JPanel();

        panel.add(oJLabel);
        panel.add(progressBar);
        mainPanel.add(panel, BorderLayout.CENTER);
	
		Debug.initializeLogger();		
		
        Debug.logMessage("getParameter : "+getParameter("host"));
        Debug.logMessage("getDocumentBase().getHost() : "+getDocumentBase().getHost());

        if(getParameter("host") != null)
          strURL = "https://" + getParameter("host") + strFileSeparator;
        else
          strURL = "https://" + getDocumentBase().getHost() + ":" + getParameter("PORT") + strFileSeparator;				// support change port by chihmou 20071205
						


        Debug.logMessage("Source URL : "+strURL);

        objFile = new File(strInstallDir);
        
        strOperatingSystem = System.getProperty("os.name");
        Debug.logMessage("Client OS : "+strOperatingSystem);
        
      }

    private void connect(boolean blnIsPwdReqd)
      {
   	  	if( !setFilePermissions(blnIsPwdReqd) )
   	  	  {
			reportPermissionError();
			Debug.logMessage("File Permissions couldn't be set");
		  }
        else if( !isInstallationCorrupted() )
		   {
		   	JSObject win = (JSObject) JSObject.getWindow(this);
		    Debug.logMessage("About to open connection window : "+strRedirectURL);
   			Debug.closeLogger();
		    win.eval(strRedirectURL);
		    blnInstallSuccess = true;
		   }
   	  	else
   	  		reportInstallationCorrupted();
      }// ~connect

    private void closeAppletWindow()
      {
      	Debug.closeLogger();
        JSObject win = (JSObject) JSObject.getWindow(this);
        win.eval("window.opener.closeInstallWindow();");
      }

    private void removeOldVersion()
      {
        File objFile = new File(strInstallDir+strJNILibrary);
        
        Debug.logMessage(objFile.getName()+ (objFile.delete() ? " was successfully deleted" : " couldn't be deleted"));
      }

	private void reportOSError()
	  {
	      panel.remove(progressBar);
	      oJLabel.setText("Installation Failed");
          JOptionPane.showMessageDialog(null, "This OS is presently not supported...!", "OS recognition error", JOptionPane.ERROR_MESSAGE);
	
	  }
	
	private void reportInstallError()
	  {
	      panel.remove(progressBar);
          oJLabel.setText("Installation Failed");
          JOptionPane.showMessageDialog(null,"Error : Check if..\n\n\t1. you have root/administrative privileges on your system \n\t2. there is sufficient disk space","Installation Failed",JOptionPane.ERROR_MESSAGE);
	  }
	  
	private void reportPermissionError()
	  {
	      panel.remove(progressBar);
          oJLabel.setText("File Permissions not set");
          JOptionPane.showMessageDialog(null, "Needed permissions couldn't be set on some files.\n\"Root\" privileges required for setting permissions.", "File Permission Error", JOptionPane.ERROR_MESSAGE);
	  }
	private void reportDownloadError()
	  {
	      panel.remove(progressBar);
          oJLabel.setText("Installation Failed");
          JOptionPane.showMessageDialog(null, "An error occured while downloading VPClient.\nUninstall and then try installing again.", "Error downloading files", JOptionPane.ERROR_MESSAGE);
	  }

	private void reportLinuxUpdateAvailable()
	  {
          JOptionPane.showMessageDialog(null, "An updated version of VPClient is available.To update, just click on the\n \"Launch\" icon after logging in with the user id used for installation.","Update Available...!", JOptionPane.INFORMATION_MESSAGE);
  	  }
	
	private void reportMacUpdateAvailable()
	  {
          JOptionPane.showMessageDialog(null, "To upgrade VPClient login with root/administrative privileges.","Upgrade Available...!", JOptionPane.INFORMATION_MESSAGE);
  	  }

	private void reportInstallationCorrupted()
	  {
	      panel.remove(progressBar);
	      oJLabel.setText("Installation Failed");
          JOptionPane.showMessageDialog(null, "VPClient has been found to be corrupted and needs to be reinstalled", "Error extracting installation files", JOptionPane.ERROR_MESSAGE);
	  }

	/*
	 * This method checks if any of the files required for running VP has been
	 * renamed or deleted and if yes reports installation has been corrupted
	 */
	private boolean isInstallationCorrupted()
	  {
	  	  boolean blnRetVal = false;
	  	  
	  	  if( strExtractFiles != null )
	  	  {
	  	  	  /* The last file in the array should be ConfigScript and
	  	  	   * is not required for launching VPClient, hence ignoring
	  	  	   */  	  
		  	  for(int index = 0; index < strExtractFiles.length-2; index++)
			  	 {
			  	 	if( new File(strInstallDir+strExtractFiles[index]).exists() )
			  	 		continue;
			  	 	else 
			  	 	  {
			  	 	  	blnRetVal = true;
			  	 	  	break;
			  	 	  }
			  	 }
	  	  }
	  	  return blnRetVal;
 	  }
	
	private boolean isLinux()
	{
		return (strOperatingSystem.toLowerCase().indexOf("linux") != -1);
	}
	
	private boolean isMac()
	{
		return (strOperatingSystem.toLowerCase().indexOf("mac") != -1);
	}

	/*
	 * This method checks if the OS on the client is supported by the VP application
	 */
	private boolean isOSSupported()
	  {
	  	boolean blnReturn = false;
	  	
		if(strOperatingSystem != null)
   		  {
	   		  if( isLinux() )
				{
					strFileName = "SignedVPClient.jar";
					strRedirectURL = "window.opener.connect_linux();";
					strExtractFiles = strLinuxInstallationFiles;
					blnReturn = true;
				}
			  else if( isMac() )
				{
					strFileName = "SignedMacVPClient.jar";
					strRedirectURL = "window.opener.connect_mac();";
				   	strExtractFiles = strMacInstallationFiles;
				   	blnReturn = true;
				}
   		  }// ~if(strOperatingSystem != null)
		
		  return blnReturn;
	  }
			
    public void start()
      {
   		if(!isOSSupported())
   		  {
			reportOSError();
   		  }
		else
		  {
		  	boolean blnIsPwdReqd = true;
	        	switch( copyFiles() )
	          	{
	            	//Error occured while creating the directory : Insufficient privilege for installation
               			case -1:
                			reportInstallError();
                        		break;

                        //Error occured while downloading the file : Connection error
                		case -2:
                			reportDownloadError();
                        		break;
                        //Intimate the user of the availability of a newer version
                		case -3:			
                        		if( isLinux() )
                        			reportLinuxUpdateAvailable();
		                  	      	connect(blnIsPwdReqd);
                 		        	break;
					
						//Password already obtained from the MAC user during upgrade
                		case -4:                		
	               			blnIsPwdReqd = false;

                       //File(s) were successfully downloaded
                		default:
                        		if( !new Extracter().extractFiles(strInstallDir+strFileName, strExtractFiles, strInstallDir) )
                          		{
                          			reportInstallationCorrupted();
                            			break;
                          		}
                         		else
                           			oJLabel.setText("File successfully downloaded..!");
	                        		/* Don't break */

                        //Files were already existing and none had to be downloaded
                		case 0:
                      			connect(blnIsPwdReqd);
                        		break;
	          	}// ~switch
		  }// ~else

       	if( !blnInstallSuccess )
       		closeAppletWindow();
      }



	public void destroy()
	{
		Debug.closeLogger();
	}
	
	public void stop()
	{
		Debug.closeLogger();
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
	}
	
	private String getPassword(String strMessage)
 	  {
 	  	Object objOptions[] = {"Ok","Cancel"};

		JPanel objPanel = new JPanel();
	  	JPasswordField objPwdField = new JPasswordField();
	  	JLabel objLabel = new JLabel(strMessage, JLabel.CENTER);
	  	objPanel.setLayout(new BorderLayout(0,0));
	  	objPanel.add(objLabel,BorderLayout.CENTER);
	  	objPanel.add(objPwdField,BorderLayout.SOUTH);
		objPanel.setVisible(true);
	  	objPwdField.requestFocus();

 	    int iRetVal;
 	    
		iRetVal = 
			JOptionPane.showOptionDialog(null,
										objPanel,
										"Authentication Required",
 	  									JOptionPane.DEFAULT_OPTION,
 	  									JOptionPane.PLAIN_MESSAGE,
 	  									null,
 	  									objOptions,
 	  									objOptions[0]);
 	    	

 	  	return new String(objPwdField.getPassword());
	  }

	private void writeStream(Process child, String strInput)
	  {
      	StringBuffer buffer = new StringBuffer();
       	String temp = null;
	            	
       	try
       	{
	       	BufferedWriter objBIS = 
	       					new BufferedWriter(new OutputStreamWriter(child.getOutputStream()));
			objBIS.write(strInput);      
			objBIS.flush();
			/*
			 * This causes sudo to fail on first incorrect entry of 
			 * password, otherwise it will offer two more chances
			 */
	       	objBIS.close(); 
       	}
       	catch(Exception objE)
       	{
	       	Debug.logMessage(objE.toString());
       	}
	  }
	
	private int execSystem(String []strCommand,boolean isInputExpected, String strUserPrompt)
	  {
	    int retVal = -1;
	    String strInput = null;
        try
          {
            Runtime objRuntime = Runtime.getRuntime();
            logParameters(strCommand);
            if( isInputExpected )
              {
            	strInput = getPassword(strUserPrompt);

           		/*
           		 * To avoid creation of a process for which  
           		 * input is required but not available...!
           		 */
	           	if( strInput == null || strInput.trim().length() == 0)
            		return retVal;
              }
            
            Process child =  objRuntime.exec(strCommand);
            if( isInputExpected )
              {
		        writeStream(child,strInput);
              }
            Debug.logMessage(readStream(child,true));	//read Error stream
            Debug.logMessage(readStream(child,false));	//read Output stream
            child.waitFor();
            retVal = child.exitValue();
            Debug.logMessage(strCommand[0]+" exited with "+retVal);
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

    private boolean setFilePermissions(boolean blnIsPwdReqd)
      {
      	String strConfigScript = null;
      	boolean blnIsSuccess = true;
      	File objFile = null;

        if( isLinux() )
        	strConfigScript = strLinuxConfigScript;
        else if( isMac() )
        	strConfigScript = strMacConfigScript;

   	  	objFile = new File(strInstallDir+strConfigScript);
		
   	  	if( objFile.exists() )
   	  	  {
			String strPwdPrompt = "Enter your password to complete installation";
			String strCmd1 = "chmod +x "+strInstallDir+strConfigScript;
		
			boolean blnIsInputNeeded = isMac() & blnIsPwdReqd;
	
			String strCmd2 = strInstallDir+strConfigScript+" "+strInstallDir+" "+blnIsInputNeeded;

	    	if(execSystem(makeCommand(strCmd1), false, null)!= 0 ||
		   	   execSystem(makeCommand(strCmd2), blnIsInputNeeded, strPwdPrompt)!= 0)
	      	  {
   				blnIsSuccess = false;
       		  }
       		else
				/* Delete the script file after executing it */
   				objFile.delete();
   	  	  }

        return blnIsSuccess;
      }// ~setFilePermissions

	private boolean canUpgradeContinue()
	  {
	  	String strPwdPrompt = "Upgrade available..!Click 'Cancel' to retain the existing version.";
	  	String strCmd[] =
	  	{
	  		"bash",
	  		"-c",
	  		"sudo -K;sudo -S -u root chmod ugo-x "+strInstallDir+strPPPWrap+" "+strInstallDir+strRouteWrap
	  	};
	  	return (execSystem(strCmd, true, strPwdPrompt) == 0);
	  }
	  
    private boolean isUpgradeAvailable(String srcURL, String destFile)
      {
        boolean blnisUpgradeAvailable = true;
        JarFile objClientJar = null;
        Manifest objManifest = null;
        String strVersion = null;
        try
          {
			objClientJar = new JarFile(destFile);
			
			if(objClientJar.getManifest() == null)
			  {
			  	Debug.logMessage("Could not find Manifest file");
			  	return blnisUpgradeAvailable;
			  }

			objManifest = objClientJar.getManifest();

			if(objManifest.getAttributes("Virtual Passage") == null)
			  {
			  	Debug.logMessage("Could not read Entry");
			  	return blnisUpgradeAvailable;
			  }

			strVersion = (objManifest.getAttributes("Virtual Passage")).getValue("Version");

			if( strVersion == null)
			  {
			  	Debug.logMessage("Could not read Version");
			  	return blnisUpgradeAvailable;
			  }
			
			blnisUpgradeAvailable = !(strVersion.equals(getParameter("jar_version")));
          }
        catch(Exception objE)
          {
            Debug.logMessage(objE.toString());
          }

       	Debug.logMessage("Server Jar Version : "+getParameter("jar_version"));
       	Debug.logMessage("Client Jar Version : "+strVersion);

	  	Debug.logMessage("Update Available : "+blnisUpgradeAvailable);

        return blnisUpgradeAvailable;
      }

    private int copyFiles() 
      {
        int no_of_files_copied = 0;

        Debug.logMessage(strInstallDir+" exists : "+objFile.exists());    
        if(!objFile.exists())
          {
          	try
          	  {
	            if(!objFile.mkdir())
	              {
	                strInstallDir = null;
	                no_of_files_copied = -1;
	              }
          	  }
          	catch(Exception objE)
          	  {
          	  	Debug.logMessage(objE.toString());
          	  }
          }

        Debug.logMessage("Installation Directory : "+strInstallDir);   

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
                if( isUpgradeAvailable(strURL + strFileName, strInstallDir + strFileName) )
                  {
                    if( !objFile.canWrite() || (isMac() && !canUpgradeContinue()) )
                      no_of_files_copied = -3 ;
                    else
                      {
                        //If download of latest version succeeds ...
                        if( copyURL(strURL+strFileName, strInstallDir+strFileName) == 0 )
                          {
				if( isMac() )
					no_of_files_copied = -4;//Password already obtained
				else
		                        no_of_files_copied++ ;

                            removeOldVersion();
                          }
                        else
                          	no_of_files_copied = -2;
                      }

                  }// ~if(isUpgradeAvailable....)

              }// ~else

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
            Debug.logMessage("Opening connection to " + srcURL + "...");
            URLConnection urlC = objURL.openConnection();

            Debug.logMessage("URL.openStream . . .");
            InputStream is = objURL.openStream();

            Debug.logMessage("Resource type : " + urlC.getContentType());
            Date date=new Date(urlC.getLastModified());
            Debug.logMessage("Last modified on : " + DateFormat.getDateInstance().format(date));
            file_size = urlC.getContentLength();
            Debug.logMessage("File size : "+file_size);
            System.out.flush();

            FileOutputStream fos=null;
            fos = new FileOutputStream( new File(destFile) );
            int bytes_read, count=0;

            Debug.logMessage("Copying " + srcURL +" to "+ destFile);

            while ((bytes_read=is.read()) != -1)
              {
                fos.write(bytes_read);
                count ++;
                progressBar.setValue(count*100/file_size);
              }
            is.close();
            fos.close();

            Debug.logMessage(count + " byte(s) copied");
          }// ~try
        catch(Exception objE)
          { 
            retVal = -1;
            Debug.logMessage(objE.toString());
          }
        return retVal;
      }//~copyURL

}// ~Installer

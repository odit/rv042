import java.io.File;

import com.cavium.Debug;
import java.awt.Container;
import java.awt.BorderLayout;
import javax.swing.JPanel; 
import javax.swing.JLabel;
import javax.swing.JApplet;
import javax.swing.JOptionPane;
import javax.swing.JPasswordField;

import java.io.BufferedReader;
import java.io.BufferedWriter;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

import netscape.javascript.JSObject;

/* 
 * This class looks for the files installed under the installation
 * directory and cleans them up .Sufficient file permissions are needed
 * to perform this operation and appropriate messages are thrown on failure
 * to do so.
 */
public class UnInstaller extends JApplet 
{
    static final long serialVersionUID = 0;

    private String strFileSeparator = System.getProperty("file.separator");
    private String strInstallDir = "/vpclient"+strFileSeparator;
    private String strLogDir = "/tmp"+strFileSeparator;
	private String strUninstallScript = strInstallDir+"uninstaller.sh";

	private String strOperatingSystem = null;
	
    private Container mainPanel = null;
    private JPanel panel = null;
    private JLabel oJLabel = null;

    private File objFile = null;	
    
    private final int UNINSTALL_SUCCESS = 0;
    private final int ALREADY_UNINSTALLED = -1;
    private final int UNINSTALLER_CORRUPTED = -2;
    private final int UNINSTALLATION_CANCELLED = -3;
    private final int PERMISSION_DENIED = -4;
    private final int INCOMPLETE_UNINSTALL = -5;
    
    public void init()
      {
        mainPanel = getContentPane();

        oJLabel = new JLabel("UnInstalling Files....!");
        oJLabel.setSize(450,200);
        panel = new JPanel();

        panel.add(oJLabel);
        mainPanel.add(panel, BorderLayout.CENTER);

		Debug.initializeLogger();		
        strOperatingSystem = System.getProperty("os.name");
        Debug.logMessage("Client OS : "+strOperatingSystem);
      }

    private void closeAppletWindow()
      {
      	Debug.closeLogger();
        JSObject win = (JSObject) JSObject.getWindow(this);
        win.eval("window.opener.closeUninstallWindow();");
      }

	private boolean isMac()
	{
		return (strOperatingSystem.toLowerCase().indexOf("mac") != -1);
	}
	
    public void start()
      {
      	String strCmd[] = {strUninstallScript, strInstallDir};
      	
        switch(isMac() ? execSystem(strCmd, true,"Enter your password to uninstall"):deleteFiles())
          {
                case UNINSTALL_SUCCESS:	//Uninstall succesful
                        oJLabel.setText("Uninstallation completed successfully");
                        JOptionPane.showMessageDialog(null,"VPClient has been successfully uninstalled from your machine","Uninstallation completed successfully",JOptionPane.PLAIN_MESSAGE);
                        break;

				case ALREADY_UNINSTALLED:
                        oJLabel.setText("Uninstallation aborted");
                        JOptionPane.showMessageDialog(null,"VPClient may have already been uninstalled or renamed.","Uninstallation aborted",JOptionPane.PLAIN_MESSAGE);
                        break;

				case UNINSTALLER_CORRUPTED:
                        oJLabel.setText("Uninstallation failed");
                        JOptionPane.showMessageDialog(null,"VPClient uninstaller is corrupted","Uninstallation failed",JOptionPane.ERROR_MESSAGE);
                        break;
				                       
                case UNINSTALLATION_CANCELLED:
                		break;

				case INCOMPLETE_UNINSTALL:
                        oJLabel.setText("Uninstallation failed");
                        JOptionPane.showMessageDialog(null,"Some file(s)/folder couldn't be removed.","Uninstallation failed",JOptionPane.ERROR_MESSAGE);
                        break;
						
                default:	//Insufficient privileges
                        oJLabel.setText("Uninstallation failed");
                        JOptionPane.showMessageDialog(null,"Uninstallation failed.Check your administrative privileges","Uninstallation failed",JOptionPane.WARNING_MESSAGE);
                        break;
          }// ~switch
          
          closeAppletWindow();
      }// ~start

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

	private int execSystem(String []strCommand,boolean isInputExpected, String strUserPrompt)
	  {
	    int retVal = ALREADY_UNINSTALLED;
	    String strInput = null;
	    
        try
          {
          	File objFile = new File(strCommand[0]);
          	if( objFile.exists() )
          	  {
	            Runtime objRuntime = Runtime.getRuntime();
	            logParameters(strCommand);
	            Process child =  objRuntime.exec(strCommand);

	            if( isInputExpected )
	              {
	            	strInput = getPassword(strUserPrompt);
	
		           	if( strInput == null || strInput.trim().length() == 0)
		           	  {
	            		retVal = UNINSTALLATION_CANCELLED;	// Password was not entered 
	            		child.destroy();	// Kill the process
	            	  }
	              	else
				        writeStream(child,strInput);
	              }
	            if(retVal != UNINSTALLATION_CANCELLED)	//Don't wait for a destoryed process...!
				  {
					Debug.logMessage(readStream(child,false));
		            child.waitFor();
		            retVal = child.exitValue();
		            Debug.logMessage(strCommand[0]+" exited with "+retVal);
		
		            if(retVal != 0)
			           	Debug.logMessage("From error stream\n\t\t\t"+readStream(child,true));
				  }
	          }
	      }    
	    catch(IOException objE)
	      {
	      	retVal = UNINSTALLER_CORRUPTED;
	      	Debug.logMessage(objE.toString());
	      }
	    catch(Exception objE)
	      {
	      	Debug.logMessage(objE.toString());
	      }

        return retVal;
	  }

    public int deleteFiles() 
      {
        int no_of_files_deleted = ALREADY_UNINSTALLED;
        objFile = new File(strInstallDir);
        try
          {
            Debug.logMessage(strInstallDir+" exists : "+objFile.exists());    
            if(!objFile.exists())
              {
                Debug.logMessage
                ("VPClient has already been uninstalled or its folder has been 	renamed");
                strInstallDir = null;
              }
            else if( !objFile.canWrite() )
              {
                Debug.logMessage("Insufficient privilege for performing uninstallation");
                no_of_files_deleted = PERMISSION_DENIED;	//Insufficient privilege			
              }
            else
              {
                File[] objFiles = objFile.listFiles();
                if(objFiles.length > 0)	//Is the folder already empty ?
                  {
                    for(int i=0; i<objFiles.length; i++)
                      {
                        Debug.logMessage(objFiles[i].getName());
                        if(objFiles[i].delete())
                          {
                            no_of_files_deleted++;
                            Debug.logMessage(" was successfully removed");
                          }
                        else
                          Debug.logMessage(" couldn't be removed");
                      }// ~for(..)

                    if(no_of_files_deleted < objFiles.length)//some files couldn't be removed ?
                      no_of_files_deleted = INCOMPLETE_UNINSTALL;
                  }// ~if

                  if( !objFile.delete() )	//Install Directory removal fails ?
                    {
                      no_of_files_deleted = INCOMPLETE_UNINSTALL;
                      Debug.logMessage(objFile.getName()+" couldn't be removed");
                    }
                  else
                      no_of_files_deleted = UNINSTALL_SUCCESS;

              }// ~else	

            objFile = null;

          }// ~try
        catch(SecurityException objSE)
          {
            no_of_files_deleted = PERMISSION_DENIED;
            objSE.printStackTrace();
          }// ~catch

        return no_of_files_deleted;

      }// ~deleteFiles

}// ~UnInstaller

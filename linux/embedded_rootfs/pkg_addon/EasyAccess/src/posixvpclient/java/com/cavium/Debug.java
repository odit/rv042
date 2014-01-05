package com.cavium;
/* Classes needed for logging messages*/
import java.io.File;
import java.util.Date;
import java.io.FileWriter;
import java.text.DateFormat;
import java.io.BufferedWriter;

public class Debug
{
	/* variables used for logging */
	private static String strLogFile = "/tmp/jnilib.log";
	private static BufferedWriter bwLog = null;
	private static boolean isEnabled = false;
	private static Date objDate = null;
    
	public static boolean getDebugEnabled()
	{
		return isEnabled;
	}
	
	public static String getFileName()
	{
		return strLogFile;
	}
	public static void logMessage(String strMessage)
	  {
		try
		  {
			if(isEnabled)
			  {
				objDate.setTime(System.currentTimeMillis());
			   	bwLog.write(DateFormat.getInstance().format(objDate)+" -> "+strMessage+"\n");
			   	bwLog.flush();
			  }
		  }
		catch(Exception objE)
		  {
			objE.printStackTrace();
		  }
	  }
	
	public static void closeLogger()
	  {
		try
		  {
			if(isEnabled)
			{
			   bwLog.close();
			   isEnabled = false;
			}
		  	objDate = null;
		  }
		catch(Exception objE)
		  {
			objE.printStackTrace();
		  }
	  }

	public static void initializeLogger()
	  {
		try
		  {
			File objFile = new File(strLogFile);
			if( objFile.exists() && objFile.canWrite() )
			  {
				bwLog = new BufferedWriter(new FileWriter(strLogFile, true) );
				isEnabled = true;
				System.out.println("Debugging enabled");
			  }
			else
			  {
				bwLog = null;
				isEnabled = false;
			  }
			
			objFile = null;
		  	objDate = new Date();
		  }
		catch(Exception objE)
		  {
			objE.printStackTrace();
		  }
	  }
}// ~Debug

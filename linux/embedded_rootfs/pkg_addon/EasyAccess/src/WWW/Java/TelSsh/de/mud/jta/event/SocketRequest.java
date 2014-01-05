/*
 * This file is part of "The Java Telnet Application".
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * "The Java Telnet Application" is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
package de.mud.jta.event;

import de.mud.jta.PluginMessage;
import de.mud.jta.PluginListener;
import de.mud.jta.event.SocketListener;

public class SocketRequest implements PluginMessage {
	
	String tunnelServer;
	String session;
  	String host;
	int port;
        String service;
	int https_port;  //Rain

  /** Create a new disconnect message */
  public SocketRequest() {
	  tunnelServer = null;
	  session = null;
	  host = null;
  }

  /** Create a new connect message */
  //Rain -->
  //public SocketRequest(String tunnelServer, String session, String host, int port, String service) {
  public SocketRequest(String tunnelServer, String session, String host, int port, String service, int https_port) {
  //<-- Rain
	  this.tunnelServer = tunnelServer;
	  this.session = session;
	  this.host = host;
	  this.port = port;
	  this.service = service;
	  this.https_port = https_port;  //Rain
  }

  /**
   * Tell all listeners that we would like to connect.
   * @param pl the list of plugin message listeners
   * @return the terminal type or null
   */
  public Object firePluginMessage(PluginListener pl) {
    if(pl instanceof SocketListener) try {
      if(host != null)
	  	//Rain -->
        //((SocketListener)pl).connect(tunnelServer, session, host, port, service);
		((SocketListener)pl).connect(tunnelServer, session, host, port, service, https_port);
		//<-- Rain
      else
        ((SocketListener)pl).disconnect();
    } catch(Exception e) { 
      e.printStackTrace();
    }
    return null;
  }
}

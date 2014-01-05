package org.bouncycastle.asn1.x509;

import java.io.*;
import java.util.*;

import org.bouncycastle.asn1.*;

public class X509Name
    implements DEREncodable
{
    /**
     * country code - StringType(SIZE(2))
     */
    public static final DERObjectIdentifier C = new DERObjectIdentifier("2.5.4.6");

    /**
     * organization - StringType(SIZE(1..64))
     */
    public static final DERObjectIdentifier O = new DERObjectIdentifier("2.5.4.10");

    /**
     * organizational unit name - StringType(SIZE(1..64))
     */
    public static final DERObjectIdentifier OU = new DERObjectIdentifier("2.5.4.11");

    /**
     * Title
     */
    public static final DERObjectIdentifier T = new DERObjectIdentifier("2.5.4.12");

    /**
     * common name - StringType(SIZE(1..64))
     */
    public static final DERObjectIdentifier CN = new DERObjectIdentifier("2.5.4.3");

    /**
     * device serial number name - StringType(SIZE(1..64))
     */
    public static final DERObjectIdentifier SN = new DERObjectIdentifier("2.5.4.5");

    /**
     * locality name - StringType(SIZE(1..64))
     */
    public static final DERObjectIdentifier L = new DERObjectIdentifier("2.5.4.7");

    /**
     * state, or province name - StringType(SIZE(1..64))
     */
    public static final DERObjectIdentifier ST = new DERObjectIdentifier("2.5.4.8");


    /**
     * email address (RSA PKCS#9 extension) - IA5String
     * <p>
     * note: if you're trying to be ultra orthodox, don't use this! It shouldn't be in here.
     */
    public static final DERObjectIdentifier EmailAddress = new DERObjectIdentifier("1.2.840.113549.1.9.1");
	
	/**
	 * email address in Verisign certificates
	 */
	public static final DERObjectIdentifier E = EmailAddress;
	
    /*
     * others...
     */
    public static final DERObjectIdentifier DC = new DERObjectIdentifier("0.9.2342.19200300.100.1.25");

    /**
     * LDAP User id.
     */
    public static final DERObjectIdentifier UID = new DERObjectIdentifier("0.9.2342.19200300.100.1.1");

    /**
     * look up table translating OID values into their common symbols.
     */
    public static Hashtable OIDLookUp = new Hashtable();

    /**
     * look up table translating common symbols into their OIDS.
     */
    public static Hashtable SymbolLookUp = new Hashtable();

    static
    {
        OIDLookUp.put(C, "C");
        OIDLookUp.put(O, "O");
        OIDLookUp.put(T, "T");
        OIDLookUp.put(OU, "OU");
        OIDLookUp.put(CN, "CN");
        OIDLookUp.put(L, "L");
        OIDLookUp.put(ST, "ST");
        OIDLookUp.put(SN, "SN");
        OIDLookUp.put(EmailAddress, "E");
        OIDLookUp.put(DC, "DC");
        OIDLookUp.put(UID, "UID");

        SymbolLookUp.put("c", C);
        SymbolLookUp.put("o", O);
        SymbolLookUp.put("t", T);
        SymbolLookUp.put("ou", OU);
        SymbolLookUp.put("cn", CN);
        SymbolLookUp.put("l", L);
        SymbolLookUp.put("st", ST);
        SymbolLookUp.put("sn", SN);
        SymbolLookUp.put("emailaddress", E);
        SymbolLookUp.put("dc", DC);
        SymbolLookUp.put("e", E);
        SymbolLookUp.put("uid", UID);
    }

    private Vector                  ordering = new Vector();
    private Vector                  values = new Vector();
    private ASN1Sequence            seq;

    public static X509Name getInstance(
        ASN1TaggedObject obj,
        boolean          explicit)
    {
        return getInstance(ASN1Sequence.getInstance(obj, explicit));
    }

    public static X509Name getInstance(
        Object  obj)
    {
        if (obj == null || obj instanceof X509Name)
        {
            return (X509Name)obj;
        }
        else if (obj instanceof ASN1Sequence)
        {
            return new X509Name((ASN1Sequence)obj);
        }

        throw new IllegalArgumentException("unknown object in factory");
    }

    /**
     * Constructor from ASN1Sequence
     *
     * the principal will be a list of constructed sets, each containing an (OID, String) pair.
     */
    public X509Name(
        ASN1Sequence  seq)
    {
        this.seq = seq;

        Enumeration e = seq.getObjects();

        while (e.hasMoreElements())
        {
            ASN1Set         set = (ASN1Set)e.nextElement();
            ASN1Sequence    s = (ASN1Sequence)set.getObjectAt(0);

            ordering.addElement(s.getObjectAt(0));
            values.addElement(((DERString)s.getObjectAt(1)).getString());
        }
    }

    /**
     * constructor from a table of attributes.
     * <p>
     * it's is assumed the table contains OID/String pairs, and the contents
     * of the table are copied into an internal table as part of the 
     * construction process.
     * <p>
     * <b>Note:</b> if the name you are trying to generate should be
     * following a specific ordering, you should use the constructor
     * with the ordering specified below.
     */
    public X509Name(
        Hashtable  attributes)
    {
        this(null, attributes);
    }

    /**
     * constructor from a table of attributes with ordering.
     * <p>
     * it's is assumed the table contains OID/String pairs, and the contents
     * of the table are copied into an internal table as part of the 
     * construction process. The ordering vector should contain the OIDs
     * in the order they are meant to be encoded or printed in toString.
     */
    public X509Name(
        Vector      ordering,
        Hashtable   attributes)
    {
        if (ordering != null)
        {
            for (int i = 0; i != ordering.size(); i++)
            {
                this.ordering.addElement(ordering.elementAt(i));
            }
        }
        else
        {
            Enumeration     e = attributes.keys();

            while (e.hasMoreElements())
            {
                this.ordering.addElement(e.nextElement());
            }
        }

        for (int i = 0; i != this.ordering.size(); i++)
        {
            DERObjectIdentifier     oid = (DERObjectIdentifier)this.ordering.elementAt(i);

            if (OIDLookUp.get(oid) == null)
            {
                throw new IllegalArgumentException("Unknown object id - " + oid.getId() + " - passed to distinguished name");
            }

            if (attributes.get(oid) == null)
            {
                throw new IllegalArgumentException("No attribute for object id - " + oid.getId() + " - passed to distinguished name");
            }

            this.values.addElement(attributes.get(oid)); // copy the hash table
        }
    }

    /**
     * takes two vectors one of the oids and the other of the values.
     */
    public X509Name(
        Vector  ordering,
        Vector  values)
    {
        if (ordering.size() != values.size())
        {
            throw new IllegalArgumentException("ordering vector must be same length as values.");
        }

        for (int i = 0; i < ordering.size(); i++)
        {
            this.ordering.addElement(ordering.elementAt(i));
            this.values.addElement(values.elementAt(i));
        }
    }

    /**
     * takes an X509 dir name as a string of the format "C=AU, ST=Victoria", or
     * some such, converting it into an ordered set of name attributes.
     */
    public X509Name(
        String  dirName)
    {
        X509NameTokenizer   nTok = new X509NameTokenizer(dirName);

        while (nTok.hasMoreTokens())
        {
            String  token = nTok.nextToken();
            int     index = token.indexOf('=');

            if (index == -1)
            {
                throw new IllegalArgumentException("badly formated directory string");
            }

            String              name = token.substring(0, index);
            String              value = token.substring(index + 1);
            DERObjectIdentifier oid = null;

            if (name.toUpperCase().startsWith("OID."))
            {
                oid = new DERObjectIdentifier(name.substring(4));
            }
            else if (name.charAt(0) >= '0' && name.charAt(0) <= '9')
            {
                oid = new DERObjectIdentifier(name);
            }
            else
            {
                oid = (DERObjectIdentifier)SymbolLookUp.get(name.toLowerCase());
                if (oid == null)
                {
                    throw new IllegalArgumentException("Unknown object id - " + name + " - passed to distinguished name");
                }
            }

            this.ordering.addElement(oid);
            this.values.addElement(value);
        }
    }

    /**
     * return false if we have characters out of the range of a printable
     * string, true otherwise.
     */
    private boolean canBePrintable(
        String  str)
    {
        for (int i = str.length() - 1; i >= 0; i--)
        {
            if (str.charAt(i) > 0x007f)
            {
                return false;
            }
        }

        return true;
    }

    public DERObject getDERObject()
    {
        if (seq == null)
        {
            DEREncodableVector  vec = new DEREncodableVector();

            for (int i = 0; i != ordering.size(); i++)
            {
                DEREncodableVector      v = new DEREncodableVector();
                DERObjectIdentifier     oid = (DERObjectIdentifier)ordering.elementAt(i);

                v.add(oid);

                String  str = (String)values.elementAt(i);

                if (oid.equals(EmailAddress))
                {
                    v.add(new DERIA5String(str));
                }
                else
                {
                    if (canBePrintable(str))
                    {
                        v.add(new DERPrintableString(str));
                    }
                    else
                    {
                        v.add(new DERUTF8String(str));
                    }
                }

                vec.add(new DERSet(new DERSequence(v)));
            }

            seq = new DERSequence(vec);
        }

        return seq;
    }

    /**
     * test for equality - note: case is ignored.
     */
    public boolean equals(Object _obj) 
    {
        if (_obj == this)
        {
            return true;
        }

        if (_obj == null || !(_obj instanceof X509Name))
        {
            return false;
        }
        
        X509Name _oxn          = (X509Name)_obj;
        int      _orderingSize = ordering.size();

        if (_orderingSize != _oxn.ordering.size()) 
        {
			return false;
		}
		
		boolean[] _indexes = new boolean[_orderingSize];

		for(int i = 0; i < _orderingSize; i++) 
		{
			boolean _found = false;
			String  _oid   = ((DERObjectIdentifier)ordering.elementAt(i)).getId();
			String  _val   = (String)values.elementAt(i);
			
			for(int j = 0; j < _orderingSize; j++) 
			{
				if(_indexes[j] == true)
				{
					continue;
				}
				
				String _oOID = ((DERObjectIdentifier)_oxn.ordering.elementAt(j)).getId();
				String _oVal = (String)_oxn.values.elementAt(j);

                // was equalsIgnoreCase but MIDP doesn't like that.
				if(_oid.equals(_oOID) && _val.toLowerCase().equals(_oVal.toLowerCase()))
				{
					_indexes[j] = true;
					_found      = true;
					break;
				}

			}

			if(!_found)
			{
				return false;
			}
		}
		
		return true;
	}
	
    public int hashCode()
    {
        ASN1Sequence  seq = (ASN1Sequence)this.getDERObject();
        Enumeration   e = seq.getObjects();
        int           hashCode = 0;

        while (e.hasMoreElements())
        {
            hashCode ^= e.nextElement().hashCode();
        }

        return hashCode;
    }

    public String toString()
    {
        StringBuffer            buf = new StringBuffer();
        boolean                 first = true;
        Enumeration             e1 = ordering.elements();
        Enumeration             e2 = values.elements();

        while (e1.hasMoreElements())
        {
            Object                  oid = e1.nextElement();
            String                  sym = (String)OIDLookUp.get(oid);
            
            if (first)
            {
                first = false;
            }
            else
            {
                buf.append(",");
            }

            if (sym != null)
            {
                buf.append(sym);
            }
            else
            {
                buf.append(((DERObjectIdentifier)oid).getId());
            }

            buf.append("=");

            int     index = buf.length();

            buf.append((String)e2.nextElement());

            int     end = buf.length();

            while (index != end)
            {
                if ((buf.charAt(index) == ',')
                   || (buf.charAt(index) == '"')
                   || (buf.charAt(index) == '\\')
                   || (buf.charAt(index) == '+')
                   || (buf.charAt(index) == '<')
                   || (buf.charAt(index) == '>')
                   || (buf.charAt(index) == ';'))
                {
                    buf.insert(index, "\\");
                    index++;
                    end++;
                }

                index++;
            }
        }

        return buf.toString();
    }
}

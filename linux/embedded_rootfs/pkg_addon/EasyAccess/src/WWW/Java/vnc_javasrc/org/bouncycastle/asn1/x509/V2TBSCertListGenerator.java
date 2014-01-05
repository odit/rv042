package org.bouncycastle.asn1.x509;

import java.io.*;
import java.util.Vector;
import java.util.Enumeration;

import org.bouncycastle.asn1.*;

/**
 * Generator for Version 2 TBSCertList structures.
 * <pre>
 *  TBSCertList  ::=  SEQUENCE  {
 *       version                 Version OPTIONAL,
 *                                    -- if present, shall be v2
 *       signature               AlgorithmIdentifier,
 *       issuer                  Name,
 *       thisUpdate              Time,
 *       nextUpdate              Time OPTIONAL,
 *       revokedCertificates     SEQUENCE OF SEQUENCE  {
 *            userCertificate         CertificateSerialNumber,
 *            revocationDate          Time,
 *            crlEntryExtensions      Extensions OPTIONAL
 *                                          -- if present, shall be v2
 *                                 }  OPTIONAL,
 *       crlExtensions           [0]  EXPLICIT Extensions OPTIONAL
 *                                          -- if present, shall be v2
 *                                 }
 * </pre>
 *
 * <b>Note: This class may be subject to change</b>
 */
public class V2TBSCertListGenerator
{
    DERInteger version = new DERInteger(1);

    AlgorithmIdentifier     signature;
    X509Name                issuer;
    Time                    thisUpdate, nextUpdate=null;
    X509Extensions          extensions=null;
    private Vector          crlentries=null;

    public V2TBSCertListGenerator()
    {
    }


    public void setSignature(
        AlgorithmIdentifier    signature)
    {
        this.signature = signature;
    }

    public void setIssuer(
        X509Name    issuer)
    {
        this.issuer = issuer;
    }

    public void setThisUpdate(
        DERUTCTime thisUpdate)
    {
        this.thisUpdate = new Time(thisUpdate);
    }

    public void setNextUpdate(
        DERUTCTime nextUpdate)
    {
        this.nextUpdate = new Time(nextUpdate);
    }

    public void setThisUpdate(
        Time thisUpdate)
    {
        this.thisUpdate = thisUpdate;
    }

    public void setNextUpdate(
        Time nextUpdate)
    {
        this.nextUpdate = nextUpdate;
    }

    public void addCRLEntry(
        DERConstructedSequence crlEntry)
    {
        if (crlentries == null)
            crlentries = new Vector();
        crlentries.addElement(crlEntry);
    }

    public void addCRLEntry(DERInteger userCertificate, DERUTCTime revocationDate, int reason)
    {
        addCRLEntry(userCertificate, new Time(revocationDate), reason);
    }

    public void addCRLEntry(DERInteger userCertificate, Time revocationDate, int reason)
    {
        DERConstructedSequence seq = new DERConstructedSequence();
        seq.addObject(userCertificate);
        seq.addObject(revocationDate);
	
        if (reason != 0)
        {
            CRLReason rf = new CRLReason(reason);
            ByteArrayOutputStream   bOut = new ByteArrayOutputStream();
            DEROutputStream         dOut = new DEROutputStream(bOut);
            try
            {
                dOut.writeObject(rf);
            }
            catch (IOException e)
            {
                throw new IllegalArgumentException("error encoding value: " + e);
            }
	    byte[] value = bOut.toByteArray();
            DERConstructedSequence eseq = new DERConstructedSequence();
            DERConstructedSequence eseq1 = new DERConstructedSequence();
            eseq1.addObject(X509Extensions.ReasonCode);
            eseq1.addObject(new DEROctetString(value));
	    eseq.addObject(eseq1);
            X509Extensions ex = new X509Extensions(eseq);
            seq.addObject(ex);
        }
        if (crlentries == null)
            crlentries = new Vector();
        crlentries.addElement(seq);
    }

    public void setExtensions(
        X509Extensions    extensions)
    {
        this.extensions = extensions;
    }

    public TBSCertList generateTBSCertList()
    {
        if ((signature == null) || (issuer == null) || (thisUpdate == null))
        {
            throw new IllegalStateException("Not all mandatory fields set in V2 TBSCertList generator.");
        }

        DERConstructedSequence  seq = new DERConstructedSequence();

        seq.addObject(version);
        seq.addObject(signature);
        seq.addObject(issuer);

        seq.addObject(thisUpdate);
        if (nextUpdate != null)
            seq.addObject(nextUpdate);

        // Add CRLEntries if they exist
        if (crlentries != null) {
            DERConstructedSequence certseq = new DERConstructedSequence();
            Enumeration it = crlentries.elements();
            while( it.hasMoreElements() ) {
                certseq.addObject((DERConstructedSequence)it.nextElement());
            }
            seq.addObject(certseq);
        }

        if (extensions != null)
        {
            seq.addObject(new DERTaggedObject(0, extensions));
        }

        return new TBSCertList(seq);
    }
}

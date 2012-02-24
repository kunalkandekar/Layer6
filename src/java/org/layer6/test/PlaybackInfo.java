package org.layer6.test;

import javax.swing.*;
import java.io.Serializable;
import java.nio.ByteBuffer;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.io.ObjectInputStream;
import java.io.InputStream;
import java.io.IOException;
//import com.concert.util.SerializationUtil;

/**
 *
 */
public class PlaybackInfo implements Serializable {

    private String videoName;
    private int currentFrameLocation;
    private int maxFrameLocation;
    private byte[] image;
    private long generatedTime;
    private long ttl;
    private String url;
    private boolean reportDisplay;
    private ImageIcon imageIcon;

    //for updating user info
    public double locnX;
    public double locnY;

    private String trackID;
    
    public PlaybackInfoData [] pbinfo;
    
    public void setXY(double x, double y) {
    	this.locnX = x;
    	this.locnY = y;
    }
    
    public double getX() {
    	return locnX;
    }

    public double getY() {
    	return locnY;
    }

    public String getVideoName() {
        return videoName;
    }

    public int getCurrentFrameLocation() {
        return currentFrameLocation;
    }

    public int getMaxFrameLocation() {
        return maxFrameLocation;
    }

    public byte[] getImage() {
        return image;
    }

    public void setTTL(long t) {
        ttl = t;
    }
    
    public long getTTL() {
        return ttl;
    }

    public long getGeneratedTime() {
        return generatedTime;
    }

    public void setVideoName( String videoName ) {
        this.videoName = videoName;
    }

    public void setCurrentFrameLocation( int currentFrameLocation ) {
        this.currentFrameLocation = currentFrameLocation;
    }

    public void setMaxFrameLocation( int maxFrameLocation ) {
        this.maxFrameLocation = maxFrameLocation;
    }

    public void setImage( byte[] image ) {
        this.image = image;
    }

    public void setURL( String u ) {
        this.url = u;
    }

    public String getURL() {
        return this.url;
    }
    
    public void setGeneratedTime( long generatedTime ) {
        this.generatedTime = generatedTime;
    }

    public boolean isReportDisplay() {
        return reportDisplay;
    }

    public void setReportDisplay( boolean reportDisplay ) {
        this.reportDisplay = reportDisplay;
    }

    public ImageIcon getImageIcon() {
        return imageIcon;
    }

    public void setImageIcon( ImageIcon imageIcon ) {
        this.imageIcon = imageIcon;
    }


    public String getTrackID() {
        return trackID;
    }

    public void setTrackID( String trackID ) {
        this.trackID = trackID;
    }
    
    public String getFrameName() {
    	if(videoName == null) {
    		return null;
    	}
    	return videoName + "-" + currentFrameLocation + ".jpg";
    }

    public String toString() {
        return "PlaybackInfo: videoName=" + videoName + ", current=" + currentFrameLocation +
                ", max="+ maxFrameLocation + ", generatedTime=" + generatedTime + ", reportDisplay=" + reportDisplay +
                ", trackID=" + trackID + ", url=" + url + ", locnX=" + locnX + ", locnY=" + locnY;
    }
    
    public int getByteSize() {
    	int size = 0;
    	size += (4 * 2);	//2 ints - currentFrameLocation, maxFrameLocation;
    	size += (8 * 2);			//2 longs - generatedTime, TTL
    	size += (1);		//1 boolean - reportDisplay
    	size += (8 * 2);	//2 doubles - locnX, locnY
    	size+= (2 + (videoName == null ? 0 : videoName.length()));	//videoName string + short indicating strlen(videoName)
    	size+= (2 + (trackID == null ? 0 : trackID.length()));		//trackID string + short indicating strlen(trackID)
    	size+= (2 + (url == null ? 0 : url.length()));			//url string + short indicating strlen(url)
    	size+= (4 + (image == null ? 0 : image.length));		//image byte + int indicating number of bytes
    	//size+= (12 * 2); 	//2 byte overhead of java serialization
        return size;
    }
}

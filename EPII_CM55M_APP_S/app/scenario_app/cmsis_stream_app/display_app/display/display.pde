import processing.serial.*;
import java.util.Base64;
import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.awt.Image;

import java.io.InputStream;
import java.io.ByteArrayInputStream;

int lf = 10;    // Linefeed in ASCII
String myString = null;

float freq = 400000000;

Serial myPort;  // The serial port

void setup() {
  size(640, 480);
  // List all the available serial ports
  printArray(Serial.list());
  // Open the port you are using at the rate you want:
  myPort = new Serial(this, "COM9", 921600);
  myPort.clear();
  // Throw out the first reading, in case we started reading 
  // in the middle of a string from the sender.
  myString = myPort.readStringUntil(lf);
  myString = null;
  
  textSize(32);
}

void draw() {
  JSONArray click=null;
  while (myPort.available() > 0) {
    myString = myPort.readStringUntil(lf);
    if (myString != null) {
      //print("--->" + myString);
        try {
          if (myString.charAt(1)=='{')
          {
            background(0);
  fill(255,255,255);
          JSONObject json = parseJSONObject(myString);
          if (json!=null)
          {
             //println(myString);
             JSONObject data = json.getJSONObject("data");
             if (data != null)
             {
               click = data.getJSONArray("algo_tick");
               String image = data.getString("image");
               if (image != null)
               {
             
                 //print(image);
                 byte[] decoded = Base64.getDecoder().decode(image);
                 InputStream is = new ByteArrayInputStream(decoded);
                 BufferedImage newBi = ImageIO.read(is);
                 PImage p=new PImage((Image)newBi);
                 p.resize(640, 480);
                 image(p, 0, 0);
               } 
               
               
               if (click != null)
               {
                 float ms = 1.0*click.getJSONArray(0).getInt(0)/freq*1000;
                 text(str(ms) + " ms",5,40);
               }
               
               
              
             }
          }
          }
        }
        catch (RuntimeException e) {
          //text("Err:" + e.toString() ,5,80);
        }
        catch(IOException ioe)
        {
        //  background(0);
        //  fill(255,255,255);
         
        //   text("Err:" + ioe.toString() ,5,80);
       }
    }
  }
}

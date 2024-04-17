import processing.serial.*;
import java.util.Base64;
import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import java.awt.Image;

import java.io.InputStream;
import java.io.ByteArrayInputStream;

int lf = 10;    // Linefeed in ASCII
String myString = null;

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
}

void draw() {
  while (myPort.available() > 0) {
    myString = myPort.readStringUntil(lf);
    if (myString != null) {
      //print("--->" + myString);
        try {
          if (myString.charAt(1)=='{')
          {
          JSONObject json = parseJSONObject(myString);
          if (json!=null)
          {
             //println(myString);
             JSONObject data = json.getJSONObject("data");
             if (data != null)
             {
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
              
             }
          }
          }
        }
        catch (RuntimeException e) {
        }
        catch(IOException ioe)
        {
        }
    }
  }
}
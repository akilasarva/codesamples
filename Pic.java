/* Code for image analysis and augmentation - used for culvert research */

import java.awt.*;
import java.awt.Frame;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.font.*;
import java.awt.geom.*;
import java.awt.image.BufferedImage;
import java.text.*;
import java.util.*;
import java.util.List;
import javax.swing.*;

public class Pic extends SimplePicture {
  ///////////////////// constructors //////////////////////////////////

  /**
   * Constructor that takes a file name and creates the picture 
   * @param fileName the name of the file to create the picture from
   */
  public Pic(String fileName) {
    super(fileName);
  }
  
  /**
   * Constructor that takes the width and height
   @param height the height of the desired picture
   @param width the width of the desired picture
   */
  public Pic(int height, int width) {
    //parent class handles width and height
    super(width,height);
  }
  
  /**
   * Constructor that takes a picture and creates a copy of that picture
   @param copyPicture the picture to copy
   */
  public Pic(Picture copyPicture) {
    //parent class takes the copy
    super(copyPicture);
  }
  
  /**
   * Constructor that takes a buffered image
   * @param image the buffered image to use
   */
  public Pic(BufferedImage image) {
    super(image);
  }
  
  ////////////////////// methods for image modification + preprocessing ///////////////////////////////////////
  
 /** Method to turn entire image grayscale to ignore color in background */
 public void grayscale() { 
    Pixel[][] pixels = this.getPixels2D();
    for (Pixel[] rowArray : pixels){
        for (Pixel pixelObj : rowArray){
	    int avg = (int) (0.299*pixelObj.getRed() + 0.587*pixelObj.getGreen() + 0.114*pixelObj.getBlue());
	    pixelObj.setBlue(pixelObj.getRed());
	    pixelObj.setGreen(pixelObj.getRed());
        }
    }
  }
  
  /** Method to shrink entire image for better runtimes 
  @return newpic smaller version of original image
  */
  public Picture smaller() { 
    Picture newpic = new Picture(this.getPixels2D().length/2+1,this.getPixels2D()[0].length/2+1);
    Pixel[][] pixels = this.getPixels2D();
    Pixel[][] smallPixels = newpic.getPixels2D();
    for (int row = 0; row < pixels.length; row+=2){
        for (int col = 0; col < pixels[0].length; col+=2){}
            smallPixels[row/2][col/2].setColor(pixels[row][col].getColor());
        }
    }
    return newpic;
  }
  
/** Method to pixelate image for contrast in clarity within the image */
  public void pixelate() {
    Pixel[][] pixels = this.getPixels2D();
    int r,g,b;
    Color c;
    for(int i=4; i<pixels.length-8; i+=8){
      for (int j=4; j<pixels[0].length-8; j+=8){
	  r=0;
	  g=0;
	  b=0;
	  for(int k=-4; k<=4; k++){
	    for (int m =-4; m<=4; m++){
	      r+=pixels[i+k][j+m].getRed();
	      g+=pixels[i+k][j+m].getGreen();
	      b+=pixels[i+k][j+m].getBlue();
	    }
	  }
	  c = new Color(r/81,g/81,b/81);

	  for(int k= -4; k<=4; k++){
	    for (int m=-4; m<=4; m++){
		pixels[i+k][j+m].setColor(c);
	    }
	  }
      }
    }
  }
  
  /** copy a subimage from the passed picture to the specified row and column in the current picture
    @param fromPic the picture to copy from
    @param startRow the start row to copy to
    @param startCol the start col to copy to
    */
  public void copy(Picture fromPic, int startRow, int startCol) {
    Pixel fromPixel = null;
    Pixel toPixel = null;
    Pixel[][] toPixels = this.getPixels2D();
    Pixel[][] fromPixels = fromPic.getPixels2D();
    for (int fromRow = 0, toRow = startRow; fromRow < fromPixels.length && toRow < toPixels.length; fromRow++, toRow++){
      for (int fromCol = 0, toCol = startCol; fromCol < fromPixels[0].length && toCol < toPixels[0].length; fromCol++, toCol++){
        fromPixel = fromPixels[fromRow][fromCol];
        toPixel = toPixels[toRow][toCol];
        toPixel.setColor(fromPixel.getColor());
      }
    }   
  }

  /** Method to show distinct changes of color in an image.
    @param edgeDist thickness for a space to constitute an edge
    */
  public void edgeDetection(int edgeDist) {
    Pixel leftPixel = null;
    Pixel rightPixel = null;
    Pixel[][] pixels = this.getPixels2D();
    Color rightColor = null;
    for (int row = 0; row < pixels.length; row++){
      for (int col = 0; col < pixels[0].length-1; col++){
        leftPixel = pixels[row][col];
        rightPixel = pixels[row][col+1];
        rightColor = rightPixel.getColor();
        if (leftPixel.colorDistance(rightColor) > edgeDist)
          leftPixel.setColor(Color.BLACK);
        else
          leftPixel.setColor(Color.WHITE);
      }
    }
  }
  
  /** Method that adds two images together to create a combined version.
    @param other the other picture used in the combination
    */
  public void combine(Picture other) {
    Pixel [][]pixels = this.getPixels2D();
    Pixel [][]otherpixels = other.getPixels2D();

    for(int r=0; r<pixels.length; r++){
        for(int c=0; c<pixels[0].length; c++){
	    int red = 0;
	    int green=0;
	    int blue =0;
	    red+=pixels[r][c].getRed()+otherpixels[r][c].getRed();
	    green+=pixels[r][c].getGreen()+otherpixels[r][c].getGreen();
	    blue+=pixels[r][c].getBlue()+otherpixels[r][c].getBlue();
	    Color co = new Color(red/2,green/2, blue/2);
            pixels[r][c].setColor(co);
        }
     }
  }

  /** Method that replaces a certain color that appears in the original picture with the associated pixels from the passed picture. 
    @param col the color being replaced in the original image
    @param pic the picture used in the replacement
    */
  public void chromakey(Color col, Picture pic) {
    Pixel [][]pixels = this.getPixels2D();
    Pixel [][]otherpixels = pic.getPixels2D();

    for(int r=0; r<pixels.length; r++){
        for(int c=0; c<pixels[0].length; c++){
	    if(pixels[r][c].colorDistance(col)<100){
	        pixels[r][c].setColor(otherpixels[r][c].getColor());
	    }
	}
     }
  }
  
  /** Method to sharpen an image, making edges and objects clearer.*/
  public void sharpen() {
    Pixel [][]pixels = this.getPixels2D();
    for(int r=2; r<pixels.length-2;r++){
	for(int c=2; c<pixels[0].length-2;c++){
	    int sumRed=0;
	    int sumGreen=0;
	    int sumBlue=0;
	    for(int i=r-1; i<=r+1; i++){
		for(int j=c-1; j<c+1;j++){
		    if(i!=r && j!=c){
			sumRed+= pixels[i][j].getRed()*-1;
			sumGreen+= pixels[i][j].getGreen()*-1;
			sumBlue+= pixels[i][j].getBlue()*-1;
		    }
		}
	     }
	     sumRed+= pixels[r][c].getRed()*5;
	     sumGreen+= pixels[r][c].getGreen()*5;
	     sumBlue+= pixels[r][c].getBlue()*5;
	     if(sumRed<0) sumRed=0;
	     if(sumGreen<0) sumGreen=0;
	     if(sumBlue<0) sumBlue=0;
	     if(sumRed>255) sumRed=255;
	     if(sumGreen>255) sumGreen=255;
	     if(sumBlue>255) sumBlue=255;
	     Color col= new Color(sumRed, sumGreen, sumBlue);
	     pixels[r][c].setColor(col);
	 }
     }
  }
  
 /** Method to set the blue to 0 for color correction in images due to lighting inside culvert*/
  public void zeroBlue() {
    Pixel[][] pixels = this.getPixels2D();
    for (Pixel[] rowArray : pixels){
      for (Pixel pixelObj : rowArray){
        pixelObj.setBlue(0);
      }
    }
  }
  
  /** Method to calculate cumulative area of a color of interest - useful for defects identifcation in culvert
    @param ref size of reference area
    @param c the color of the pixels being counter
	@return covered_area amount of reference area covered by defect/color of interest
    */
  public double countArea(double ref, Color c) {
    Pixel[][] pixels = this.getPixels2D();
    int width = pixels.length;
    int count=0;
    for (int row = 0; row < width; row++){
        for (int col = 0; col < width; col++){
	    if(pixels[row][col].colorDistance(c)<30){
	        count++;
	    }
        }
    }
    return (count/ref);
  }
  
  /** Method calculae a baseline color for a given frame - useful for gauging defect-free reference color of culvert
  @return c ColorLevel representing the baseline value of the current frame
  */
  public ColorLevel frameValue() {
    Pixel[][] pixels = this.getPixels2D();
    int width = pixels.length;
    int r=0;
    int b=0;
    int g=0;
    for (int row = 0; row < width; row++){
        for (int col = 0; col < width; col++){
	    r+=pixels[row][col].getRed();
	    g+=pixels[row][col].getGreen();
	    b+=pixels[row][col].getBlue();
        }
     }
     ColorLevel c = new ColorLevel(r,g,b);
     return c;
  }
  
////////////////////// methods for GUI ///////////////////////////////////////

  private static class ButtonHandler implements ActionListener {
      public void actionPerformed(ActionEvent e) {
           System.exit(0);
      }
  }
  
  public static void frameGUI() {
      JPanel a = new JPanel();
      JButton b = new JButton("Exit");
      ButtonHandler listener = new ButtonHandler();
      b.addActionListener(listener);

      JPanel content = new JPanel();
      content.setLayout(new BorderLayout());
      content.add(a, BorderLayout.CENTER);
      content.add(b, BorderLayout.SOUTH);

      JFrame window = new JFrame("GUI Test");
      window.setContentPane(content);
      window.setSize(250,100);
      window.setLocation(100,100);
      window.setVisible(true);
  }
  
  public static void main(String[] args) {
      frameGUI();
      Picture reference = new Picture("reference.JPG");
      double ref = reference.countArea(1, Color.WHITE);
      System.out.println("Area of reference: \n" + reference.countArea(1,Color.WHITE)); //revert back to 70
      System.out.println();

      Picture dft1 = new Picture("tfd1.png");
      System.out.println("Area of defect in inches squared: \n" + dft1.countArea(ref,Color.BLACK)*.25);
      System.out.println();	 

// 	  System.out.println("Automated Diagnosis Testing:");	
// 	  System.out.println();	
// 	  ColorLevel c = dft1.frameValue();
// 	  ColorLevel r = reference.frameValue();
// 	  System.out.println(compareColor(r,c));
  }
}

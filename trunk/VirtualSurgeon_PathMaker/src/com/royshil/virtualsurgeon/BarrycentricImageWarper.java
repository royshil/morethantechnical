//package com.royshil.virtualsurgeon;
//
//import java.awt.BorderLayout;
//import java.awt.Point;
//import java.awt.image.BufferedImage;
//import java.io.BufferedReader;
//import java.io.File;
//import java.io.FileReader;
//import java.io.IOException;
//import java.util.ArrayList;
//import java.util.List;
//
//import javax.imageio.ImageIO;
//import javax.swing.ImageIcon;
//import javax.swing.JFrame;
//import javax.swing.JLabel;
//import javax.swing.JPanel;
//
//public class BarrycentricImageWarper {
//
//	private static Point[][] points;
//	/**
//	 * @param args
//	 * @throws IOException 
//	 */
//	public static void main(String[] args) throws IOException {
//		points = new Point[2][];
//		BufferedImage[] images = new BufferedImage[2];
//		String[] files = new String[2];
//		files[0] = "D:/lfw/Aaron_Eckhart/Aaron_Eckhart_0001";
//		files[1] = "D:/lfw/Alexandre_Daigle/Alexandre_Daigle_0001";
//		
//		for (int i = 0; i < 2; i++) {
//			images[i] = ImageIO.read(new File(files[i] + ".jpg"));
//
//			points[i] = new Point[14];
//			points[i][0] = new Point(0,0);
//			points[i][1] = new Point(images[i].getWidth(),0);
//			points[i][2] = new Point(images[i].getWidth(),images[i].getHeight());
//			points[i][3] = new Point(0,images[i].getHeight());
//			
//			File txtFile = new File(files[i] + ".txt");
//			if(txtFile.exists()) {
//				BufferedReader br = new BufferedReader(new FileReader(txtFile));
//				String line = null;
//				int k = 4;
//				while((line = br.readLine())!=null) {
//					int spPos = line.indexOf(' ');
//					String first = line.substring(0,spPos);
//					String second = line.substring(spPos+1);
//					points[i][k++] = new Point(
//									Integer.parseInt(first),
//									Integer.parseInt(second)
//									);
//				}
//			}
//		}
//		
//		//move points[1] to center around points[0]
//		Point mid0 = findMidPoint(points[0]);
//		Point mid1 = findMidPoint(points[1]);
//		Point[] movedPoints1 = new Point[14];
//		for (int i = 0; i < 4; i++) {
//			movedPoints1[i] = points[1][i];
//		}
//		for (int i = 4; i < 14; i++) {
//			movedPoints1[i] = new Point();
//			movedPoints1[i].x = points[1][i].x - mid1.x + mid0.x;
//			movedPoints1[i].y = points[1][i].y - mid1.y + mid0.y;
//		}
//		
//		int width = images[0].getWidth();
//		int height = images[0].getHeight();
//
//		Point[][] map = new Point[width][];
//		
//		for (int x = 0; x < width; x++) {
//			map[x] = new Point[height];
//			for (int y = 0; y < height; y++) {
//				//find 3 nearest
////				int[] min = findNearest3(x, y);
//				int[] tri = findTriangle(x,y);
//				
//				//find barrycentric coord
//				double[] w = findBarrynectricCoord(x, y, min);
//				
//				//find new
//				map[x][y] = findNewPointPosition(movedPoints1, x, y, min, w);
//			}
//		}
//		
//		BufferedImage newI = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);
//		
//		for (int x = 0; x < width; x++) {
//			for (int y = 0; y < height; y++) {
//				Point mapPoint = map[x][y];
//				mapPoint.x = Math.max(Math.min(mapPoint.x, width-1),0);
//				mapPoint.y = Math.max(Math.min(mapPoint.y, height-1),0);
//				
//				int rgb = images[0].getRGB(mapPoint.x, mapPoint.y);
//				newI.setRGB(x, y, rgb);
//			}
//		}
//		
//		JFrame jf = new JFrame();
//		jf.setLayout(new BorderLayout());
//		JLabel jl = new JLabel(new ImageIcon(newI));
//		jf.add(jl,BorderLayout.CENTER);
//		jf.pack();
//		jf.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
//		jf.setVisible(true);
//	}
//
//	private static int[] findTriangle(int x, int y) {
//		//TODO: 
//		/*
//		 * check: 0 - i - i+1
//		 * 		  i - i +
//		 */
//		
//		return null;
//	}
//
//	private static Point findMidPoint(Point[] _points) {
//		Point mid = new Point();
//		for (int i = 4; i < 14; i++) {
//			mid.x = mid.x + _points[i].x;
//			mid.y = mid.y + _points[i].y;
//		}
//		mid.x = (int) ((double)(mid.x) / 10.0);
//		mid.y = (int) ((double)(mid.y) / 10.0);
//		return mid;
//	}
//
//	private static Point findNewPointPosition(Point[] _points, int x, int y,	int[] min, double[] w) {
//		Point p = new Point();
//		Point point[] = {_points[min[0]],_points[min[1]],_points[min[2]]};
//		p.x = (int) ((double)(point[0].x) * w[0] +
//					(double)(point[1].x) * w[1] +
//					(double)(point[2].x) * w[2]);
//		p.y = (int) ((double)(point[0].y) * w[0] +
//					(double)(point[1].y) * w[1] +
//					(double)(point[2].y) * w[2]);
//		return p;
//	}
//
//	private static double[] findBarrynectricCoord(int x, int y, int[] min) {
//		Point point[] = {points[0][min[0]],points[0][min[1]],points[0][min[2]]};
//	
//		int x1 = point[0].x;
//		int x2 = point[1].x;
//		int x3 = point[2].x;
//		int y1 = point[0].y;
//		int y2 = point[1].y;
//		int y3 = point[2].y;
//
//		double detT = (x1 - x3) * (y2 - y3) - (x2 - x3) * (y1 - y3);
//		double l1 = ((y2-y3)*(x-x3)+(x3-x2)*(y-y3)) / detT;
//		double l2 = ((y3-y1)*(x-x3)+(x1-x3)*(y-y3)) / detT;
//		double l3 = 1 - l1 - l2;
//		
//		return new double[] {l1,l2,l3};
//	}
//
//	private static int[] findNearest3(int x, int y) {
//		int[] min = {-1,-1,-1};
//		double[] dist = {Double.MAX_VALUE,Double.MAX_VALUE,Double.MAX_VALUE};
//		Point pt = new Point(x,y);
//		for (int i = 0; i < 14; i++) {
//			double d = Math.max(1.0, points[0][i].distance(pt));
//			if(d < dist[0]) {
//				min[2] = min[1]; min[1] = min[0];
//				dist[2] = dist[1]; dist[1] = dist[0];
//				min[0] = i; dist[0] = d;
//			} else if(d < dist[1]) {
//				min[2] = min[1];
//				dist[2] = dist[1];
//				min[1] = i; dist[1] = d;
//			} else if(d < dist[2]) {
//				min[2] = i; dist[2] = d;
//			}
//		}
//		return min;
//	}
//
//}

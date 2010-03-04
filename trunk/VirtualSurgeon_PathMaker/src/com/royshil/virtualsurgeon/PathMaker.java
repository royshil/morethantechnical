package com.royshil.virtualsurgeon;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.FileDialog;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.geom.Point2D;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

import javax.imageio.ImageIO;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;

public class PathMaker {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		final List<Point> points = new ArrayList<Point>();
		final JFrame jf = new JFrame() {
			private static final long serialVersionUID = 1L;

			@Override
			public void paint(Graphics g) {
				super.paint(g);
				
				Graphics2D g2d = (Graphics2D)g;
				
				for (int i = 0; i < points.size(); i++) {
					Point p = points.get(i);
					
					if(i>0) {
						Point pm1 = points.get(i-1);
						g2d.setColor(Color.blue);
						g2d.drawLine(pm1.x, pm1.y, p.x, p.y);
					}
					
					g2d.setColor(Color.green);
					g2d.fillRect(p.x-2, p.y-2, 5, 5);
					
					g2d.setColor(Color.red);
					g2d.drawString(String.valueOf(i), p.x-2,p.y-2);
				}
			}
		};
		
		jf.addMouseListener(new MouseAdapter() {
			@Override
			public void mouseClicked(MouseEvent me) {
				super.mouseClicked(me);
				
				Point p = new Point(me.getX(),me.getY());
				points.add(p);
				
				System.out.println("clicked " + p);
				jf.repaint();
			}
		});
		
		jf.addKeyListener(new KeyAdapter() {
			@Override
			public void keyPressed(KeyEvent ke) {
				super.keyPressed(ke);
				
				if(ke.getKeyCode() == KeyEvent.VK_ESCAPE) {
					System.exit(0);
				}
			}
		});
		

		final String[] workFile = new String[1];
		workFile[0] = "D:/lfw/Alexandre_Daigle/Alexandre_Daigle_0001.jpg";
		
		final ImageIcon img = new ImageIcon(workFile[0]);
		JLabel l = new JLabel(img);
		
		jf.setLayout(new BorderLayout());
		jf.add(l,BorderLayout.CENTER);
		
		JButton jb = new JButton("Save");
		jb.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				System.out.println("save");
				
				String toWrite = workFile[0].substring(0, workFile[0].lastIndexOf(".")) + ".txt";
				System.out.println(toWrite);
				File f = new File(toWrite);
				FileWriter fw;
				try {
					fw = new FileWriter(f);
					for (int i = 0; i < points.size(); i++) {
						Point p = points.get(i);
						fw.write(p.x + " " + p.y + "\r\n");
					}
					fw.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		});
		
		JPanel jp = new JPanel(new BorderLayout());		
		jp.add(jb,BorderLayout.PAGE_END);
		
		JButton jb2 = new JButton("Clear");
		jb2.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				System.out.println("clear");
				points.clear();
				jf.repaint();
			}
		});
		jp.add(jb2,BorderLayout.PAGE_START);
		jf.add(jp,BorderLayout.PAGE_END);
		
		JButton jb1 = new JButton("Load");
		jb1.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				System.out.println("load");
				
				FileDialog fd = new FileDialog(jf,"Load image",FileDialog.LOAD);
				fd.setVisible(true);
				
				System.out.println("loda: " +fd.getFile());
				try {
					workFile[0] = fd.getDirectory() + File.separator + fd.getFile();
					img.setImage(ImageIO.read(new File(workFile[0])));
					
					points.clear();
					File txtFile = new File(workFile[0].substring(0, workFile[0].lastIndexOf(".")) + ".txt");
					if(txtFile.exists()) {
						BufferedReader br = new BufferedReader(new FileReader(txtFile));
						String line = null;
						while((line = br.readLine())!=null) {
							int spPos = line.indexOf(' ');
							String first = line.substring(0,spPos);
							String second = line.substring(spPos+1);
							points.add(
									new Point(
											Integer.parseInt(first),
											Integer.parseInt(second)
											)
								);
						}
					}
					
					jf.repaint();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		});
		jf.add(jb1,BorderLayout.PAGE_START);
		
//		jf.setSize(img.getIconWidth(), img.getIconHeight()+40);
		jf.pack();
		jf.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		jf.setVisible(true);
	}

}

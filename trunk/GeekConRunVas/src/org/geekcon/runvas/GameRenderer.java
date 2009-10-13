package org.geekcon.runvas;

import java.awt.Color;
import java.awt.geom.Rectangle2D;
import java.io.IOException;
import java.nio.FloatBuffer;
import java.util.ConcurrentModificationException;
import java.util.List;

import javax.media.opengl.GL;
import javax.media.opengl.GLDrawable;
import javax.media.opengl.GLException;
import javax.media.opengl.glu.GLU;

import org.geekcon.runvas.utils.Utils;
import org.geekcon.runvas.utils.Vector3D;

import com.sun.opengl.util.j2d.TextRenderer;
import com.sun.opengl.util.texture.Texture;
import com.sun.opengl.util.texture.TextureCoords;
import com.sun.opengl.util.texture.TextureIO;

public class GameRenderer {
 
	public static final FloatBuffer RED_FB = FloatBuffer.wrap(new float[]{1f,0f,0f,1f});
//	private static final FloatBuffer GREEN_FB = FloatBuffer.wrap(new float[]{0f,1f,0f,1f});
//	private static final FloatBuffer RED_FB_OP50 = FloatBuffer.wrap(new float[]{1f,0f,0f,.5f});
//	private static final FloatBuffer GREEN_FB_OP50 = FloatBuffer.wrap(new float[]{0f,1f,0f,.5f});
	private GameController controller;
	TextRenderer renderer;
	TextRenderer smallRenderer;
	long fps[] = {0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l,0l};
	int fpsCount = 0;
//	private WorldMeshObject world;
	private TextRenderer midRenderer;
	private Texture backTex = null;

	public GameRenderer(GameController gameController) {
		this.controller = gameController;
		renderer = new TextRenderer(new java.awt.Font("SansSerif", java.awt.Font.BOLD, 36));
//		smallRenderer = new TextRenderer(new java.awt.Font("SansSerif", java.awt.Font.PLAIN, 16));
//		midRenderer = new TextRenderer(new java.awt.Font("SansSerif", java.awt.Font.PLAIN, 22));
		
//	    world = new WorldMeshObject();
//		world.loadFromFile("cube.obj", true);
//	    world.getModel().setLocation(new Vector3D(-.35f,.15f,-.7f));
//	    world.getRenderer().setDrawWireframe(true);
//	    world.getModel().setColor(Vector3D.ones.multiply(.3f));
//	    world.getModel().setScale(.05f);
	}
	
	public void init(GL gl) {
		//Load sky texture
//		try {
//			backTex = TextureIO.newTexture(resConfig.getStream("sky_tex.png"), true, "png");
//		    backTex.setTexParameteri(GL.GL_TEXTURE_MIN_FILTER, GL.GL_LINEAR);
//		    backTex.setTexParameteri(GL.GL_TEXTURE_MAG_FILTER, GL.GL_LINEAR);
//		} catch (GLException e) {
//			e.printStackTrace();
//		} catch (IOException e) {
//			e.printStackTrace();
//		}	
		
		for (IController go : controller.getGameobjects()) {
			go.getRenderer().init(gl);
		}
//		controller.getWorld().init(gl);
	}

	public void render(GL gl,GLDrawable drawable,long diff) {
		gl.glMatrixMode(GL.GL_MODELVIEW);
		gl.glLoadIdentity();
		
//		if(controller.getMode() != GameModel.Mode.PROJECTILE_IS_FLYING &&
//				controller.getMode() != GameModel.Mode.EXPLOSION) {
//			gl.glDisable(GL.GL_LIGHT2);
//			gl.glDisable(GL.GL_LIGHT3);
//		}
		
	    float[] p = { 1f, 0f, 1f, 1f };
	    gl.glLightfv(GL.GL_LIGHT1, GL.GL_POSITION, FloatBuffer.wrap(p));

		
	    Vector3D eye = controller.getEyeLocation();
	    final GLU glu = new GLU();
	    Vector3D lookAtLocation = controller.getLookAtLocation();
	    Vector3D directionUp = controller.getCameraDirectionUp();
		glu.gluLookAt(eye.x, eye.y, eye.z, 
				lookAtLocation.x, lookAtLocation.y, lookAtLocation.z, 
				directionUp.x, directionUp.y, directionUp.z);
		
		gl.glPushMatrix();
	    	    
	    //loop all game objects and draw
	    //do it synchronized to prevent other aspects of the game to change 
	    //the list of game objects to draw - while drawing them.
	    List<IController> gameobjects = controller.getGameobjects();
	    synchronized (gameobjects) {	
			try {
				for (IController go : gameobjects) {
						go.getRenderer().setDrawWireframe(controller.isWireframe());
						go.getRenderer().glDraw(gl);
				}
			} catch (ConcurrentModificationException e) {
			}
		}

//	    //Draw 2D data
	    Utils.ViewOrtho(gl, drawable.getWidth(), drawable.getHeight());
	    render2D(gl, drawable, diff);
	    synchronized (gameobjects) {	
			for (IController go : gameobjects) {
				go.getRenderer().glDraw2D(gl,drawable.getWidth(),drawable.getHeight());
			}
		}

	    Utils.ViewPerspective(gl);
	    
	    gl.glPopMatrix();
	}

	
	private void render2D(GL gl, GLDrawable drawable, long diff) {
	    int width = drawable.getWidth();
		int height = drawable.getHeight();

		//background
	    if(backTex != null) {
	    	TextureCoords tc = backTex.getImageTexCoords();
	        float tx1 = tc.left();
	        float ty1 = tc.top();
	        float tx2 = tc.right();
	        float ty2 = tc.bottom();
		    backTex.enable();
		    backTex.bind();
		    gl.glPushMatrix();
		    gl.glTranslatef(0f, 0f, -1f);
		    gl.glDisable(GL.GL_LIGHTING);
		    gl.glBegin(GL.GL_QUADS);
		    gl.glEnable(GL.GL_BLEND);
		    gl.glBlendFunc(GL.GL_SRC_ALPHA,GL.GL_ONE_MINUS_SRC_ALPHA);
		    gl.glColor4d(1.0, 1.0, 1.0, 1.0);
		    gl.glTexCoord2f(tx1, ty1); gl.glVertex2f(0, 0);
		    gl.glTexCoord2f(tx1, ty2); gl.glVertex2f(0, height);
		    gl.glTexCoord2f(tx2, ty2); gl.glVertex2f(width, height);
		    gl.glTexCoord2f(tx2, ty1); gl.glVertex2f(width, 0);
		    gl.glEnd();
//		    gl.glDisable(GL.GL_BLEND);
		    gl.glEnable(GL.GL_LIGHTING);
		    gl.glPopMatrix();
		}
		
//	    if(controller.getMode() == GameModel.Mode.LEVEL_WON ||
//	       controller.getMode() == GameModel.Mode.GAME_OVER ||
//	       controller.getMode() == GameModel.Mode.GAME_WON ||
//	       controller.getMode() == GameModel.Mode.RESTART_MENU) {
//	    	Utils.tintRenderingView(gl, width, height);
//	    }
	    
		fps[fpsCount%fps.length] = diff;
		fpsCount = (fpsCount+1)%fps.length;
		long fpsAvg = 0l;
		for(int i=1;i<fps.length;i++) fpsAvg+=fps[i];
		if(fpsAvg > 0l) {
			fpsAvg /= fps.length;
		} else {
			fpsAvg = 1l;
		}

		/*
		midRenderer.beginRendering(width, height);
	    midRenderer.setColor(Color.black);
	    if(controller.getMode() == GameModel.Mode.PLAYER_IS_MOVING) {
	    	AbstractPlayerController playerController = controller.getModel().getCurrentPlayer();
	    	if(playerController != null) {
		    	int numMovesDone = playerController.getNumMovesDone();
		    	if(numMovesDone < playerController.getNUM_OF_MOVES_ALLOWED()) {
			    	midRenderer.draw("Press [space] to shoot", width - 250, 10);
		    	} else {
			    	midRenderer.draw("No more moves - press [space] to shoot!", width - 450, 10);
				}
				midRenderer.draw(String.format("Moves left: %d",playerController.getNUM_OF_MOVES_ALLOWED() - numMovesDone), 10, 150);
	    	}
	    } else if(controller.getMode() == GameModel.Mode.PLAYER_IS_SHOOTING) {
	    	midRenderer.draw("Aim! Use [ctrl] for power, [shift] to skip", width - 450, 10);
	    }
	    
	    if (controller.getModel().getPlayers() != null) {
	    	int numOfEnemies = controller.getModel().getPlayers().size() - 1;
	    	midRenderer.draw(String.format("Number of enemies: %d", numOfEnemies), 10, 185);
	    }
	    midRenderer.endRendering();

	    renderer.beginRendering(width, height);
	    if(controller.getMode() == GameModel.Mode.PLAYER_IS_SHOOTING) {
	    	renderer.setColor(Color.cyan);
	    	renderer.draw("Power", width - 150, 50);
	    }

	    renderer.setColor(1.0f, 0.2f, 0.2f, 0.8f);
	    
	    //FPS... very important
//	    renderer.draw(String.format("fps: %d", 1000l / fpsAvg), 10, 10);
//	    renderer.draw("Now Playing: " + controller.getModel().getCurrentPlayer().getPlayerName(), 10, 10);
	    if (controller.getMode() == GameModel.Mode.PLAYER_IS_MOVING && (controller.getModel().getCurrentPlayer() != null)) {
	    	renderer.draw("Now Playing: " + controller.getModel().getCurrentPlayer().getPlayerName(), 10, 10);
	    } else if (controller.getMode() == GameModel.Mode.PACKAGE_IS_FALLING) {
	    	renderer.setColor(Color.yellow);
	    	drawCenterAligned(renderer, width, height, "Package Is Falling");
	    }
	    
	    if (controller.getMode() == GameModel.Mode.EXPLOSION) {
	    	for (AbstractPlayerController player: controller.getModel().getPlayers()) {
//	    		System.out.println(player.getPlayerWasHit());
	    		if (player.getPlayerWasHit() != null) {
	    			drawCenterAligned(renderer, width, height, player.getPlayerWasHit() + " got a direct hit");
//	    			player.setPlayerWasHit(null);
	    		}
	    	}
//	    	System.out.println("end");
	    }

	    if(controller.getMode() == GameModel.Mode.PLAYER_IS_SHOOTING) {
		    //chosen weapon
	    	GameModel gameModel = controller.getModel();
			PlayerObjectModel playerModel = gameModel.getPlayers().get(gameModel.getCurrentPlayerObjectIndex()).getModel();
			Class<? extends AbstractWeapon> selectedWeapon = playerModel.getSelectedWeapon();
			String weaponName = WeaponFactory.weaponName.get(selectedWeapon);
			renderer.setColor(Color.cyan);
			renderer.draw(weaponName, 10, 100);
			
			renderer.setColor(Color.blue);
			if(selectedWeapon != LaserProjectileController.class) {
				renderer.draw(
						String.format("Ammo: %d", playerModel.getAmmoMap().get(selectedWeapon)),
						10, 150);
			} else {
				renderer.draw(
						String.format("Ammo: unlimited", playerModel.getAmmoMap().get(selectedWeapon)),
						10, 150);
			}
	    }
	    	    
		//health label
//	    renderer.setColor(Color.white);
//	    renderer.draw("Health", 10, height - 50);
	    
	    //notify if enemy is moving
	    if (controller.getMode() != GameModel.Mode.PROJECTILE_IS_FLYING && 
	    	controller.getMode() != GameModel.Mode.PACKAGE_IS_FALLING &&
	    	controller.getMode() != GameModel.Mode.EXPLOSION &&
	    	controller.getMode() != GameModel.Mode.LEVEL_WON &&
	    	controller.getMode() != GameModel.Mode.GAME_OVER &&
	    	controller.getModel().getCurrentPlayer() instanceof EnemyController) 
	    {
	    	renderer.setColor(Color.yellow);
	    	String enemyMovingStr = "Enemy moving";
	    	drawCenterAligned(renderer, width, height, enemyMovingStr);
		}
	    
	    if(controller.getMode() == GameModel.Mode.LEVEL_WON) {
	    	renderer.setColor(Color.MAGENTA);
	    	drawCenterAligned(renderer, width, height, String.format("Level %d won!", controller.getModel().getCurrentLevel()));
	    	drawCenterAligned(renderer, width, height-100, "Press [Enter] for next level");
	    }
	    if(controller.getMode() == GameModel.Mode.GAME_WON) {
	    	renderer.setColor(Color.MAGENTA);
	    	drawCenterAligned(renderer, width, height, "YOU WON THE SNAILS ARMY, GOOD JOB!");
	    	drawCenterAligned(renderer, width, height-100, "Press [ESC] to exit");
	    }
	    if(controller.getMode() == GameModel.Mode.GAME_OVER) {
	    	renderer.setColor(Color.RED);
	    	drawCenterAligned(renderer, width, height, "Game Over!");
	    	drawCenterAligned(renderer, width, height-100, "Press [Enter] to restart level");
	    }
	    if(controller.getMode() == GameModel.Mode.RESTART_MENU) {
	    	renderer.setColor(Color.RED);
	    	drawCenterAligned(renderer, width, height, "Press [SHIFT] to return to game");
	    	drawCenterAligned(renderer, width, height-100, "Press [Enter] to restart level");
	    }
	    
	    renderer.endRendering();
	    
	    smallRenderer.beginRendering(width, height);
	    smallRenderer.setColor(
	    		(controller.isWireframe()) ? Color.blue : Color.red
	    				);
    	smallRenderer.draw("[W]ireframe", 15, 75);
	    smallRenderer.setColor(
	    		(controller.isStickyCam()) ? Color.blue : Color.red
	    				);
	    smallRenderer.draw("[S]tickyCam", 15, 50);
	    smallRenderer.endRendering();
	    
//	    drawHealthBar(gl);
  */
	}

	private void drawCenterAligned(TextRenderer r, int width, int height, String str) {
		Rectangle2D strBnds = r.getBounds(str);
		int textHalfX = (int)strBnds.getWidth()/2;
		int textHalfY = (int)strBnds.getHeight()/2;
		r.draw(str, width/2-textHalfX, height/2-textHalfY);
	}

//	@SuppressWarnings("unused")
//	private void drawHealthBar(GL gl) {
//		gl.glEnable(GL.GL_BLEND);
//	    gl.glBlendFunc(GL.GL_SRC_ALPHA,
//				GL.GL_ONE_MINUS_SRC_ALPHA);
//	    
//	    float barH = 40f;
//	    float barW = 350f;
//	    float barX = 135f;
//	    float barY = 17f;
//	    GameModel gameModel = controller.getModel();
//	    AbstractPlayerController apc = gameModel.getCurrentPlayer();
//	    if(apc == null) return;
//		PlayerObjectModel playerModel = apc.getModel();
//	    float health = playerModel.getHealth();
//	    
//	    gl.glShadeModel(GL.GL_SMOOTH);
//	    gl.glDisable(GL.GL_LIGHTING);
//	    gl.glBegin(GL.GL_QUADS);
//	    //faded out background
//	    gl.glColor4f(.1f,.1f,.1f,.2f);
//	    gl.glVertex2f(barX, barY);
//	    gl.glVertex2f(barX+barW,barY);
//	    gl.glVertex2f(barX+barW,barY+barH);
//	    gl.glVertex2f(barX,barY+barH);
//	    
//	    gl.glColor4fv(RED_FB);
//	    gl.glVertex2f(barX, barY);
//	    gl.glColor4f(1f-health,health,0f,1f);
//	    gl.glVertex2f(barX+barW*health,barY);
//	    gl.glColor4f(1f-health,health,0f,1f);
//	    gl.glVertex2f(barX+barW*health,barY+barH);
//	    gl.glColor4fv(RED_FB);
//	    gl.glVertex2f(barX,barY+barH);
//	    gl.glEnd();
//	    
//	    gl.glDisable(GL.GL_BLEND);
//	    gl.glEnable(GL.GL_LIGHTING);
//	}
}

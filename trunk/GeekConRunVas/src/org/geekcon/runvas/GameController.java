package org.geekcon.runvas;

import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import javax.media.opengl.GL;
import javax.media.opengl.GLAutoDrawable;
import javax.media.opengl.GLException;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.KeyListener;
import org.geekcon.runvas.utils.Face;
import org.geekcon.runvas.utils.Vector3D;
import org.geekcon.runvas.utils.Vertex;

import com.sun.opengl.util.texture.Texture;
import com.sun.opengl.util.texture.TextureCoords;
import com.sun.opengl.util.texture.TextureIO;


public class GameController implements KeyListener, java.awt.event.MouseListener, MouseMotionListener,
	IRunvasObjectsController	 {
	
	private GameModel model;
	
	private GameRenderer renderer;
	
	@SuppressWarnings("unused")
	private boolean m_mouseDown;
	private int m_lastPosX;
	private int m_laspPosY;

	private boolean stickyCam = true;

	private boolean wireframe = false;
	
//	private boolean worldInit = false;

	private boolean moveBack;
	
	HashMap<Integer, Strip> runvasObjIdToStrip = new HashMap<Integer, Strip>();
	
	public Vector3D getEyeLocation() {
		return model.getEyeLocation();
	}

	public void setEyeLocation(Vector3D eyeLocation) {
		model.setEyeLocation(eyeLocation);
	}

	public GameController() {
		model = new GameModel();
		model.setLookAtLocation(new Vector3D(0f,0f,0f));
		model.getEyeLocation().z = 1f;
	    
	    renderer = new GameRenderer(this);
	    		
		try {
			UDPServerThread serverThread = new UDPServerThread();
			serverThread.rnvsObjCtrlr = this;
//			serverThread.start();
			System.out.println("Runvas server online.");
		} catch (IOException e) {
			System.err.println("Runvas server is offline!: " + e.getLocalizedMessage());
		}
	}

	@Override
	public void incomingRunvasObject(RunVasObject o) {
		System.out.println("handle incoming runvas object: "+o);
		if(runvasObjIdToStrip.containsKey(o.id)) {
			runvasObjIdToStrip.get(o.id).addToStrip(o);
		} else {
			Strip p = new Strip();
			p.addToStrip(o);
			runvasObjIdToStrip.put(o.id, p);
			model.getGameobjects().add(p);
			model.getDynamicObjects().add(p);
		}
	}

	public void drawGame(GL gl, GLAutoDrawable drawable, long diff) {
		//notify game to update the location of all objects
	    advanceDynamicObjects(diff);

//	    if(!worldInit) {
//	    	model.getWorld().init();
//	    	worldInit = true;
//	    }
	    
	    //Draw the scene
	    renderer.render(gl, drawable, diff);	    
	}


	public void advanceDynamicObjects(long diff) {
		List<IController> dynamicObjects = new ArrayList<IController>(model.getDynamicObjects());
		synchronized (dynamicObjects) {
			for (IController go : dynamicObjects) {
				go.advance(diff);
			}			
		}

	    //remove object that might need to be removed
	    model.cleanUp();

		updateCameraPosition(diff);
		
//		if(getMode() != GameModel.Mode.GAME_OVER && 
//				checkIfAllEnemiesAreDead()) {
//			if (model.getCurrentLevel() == 5) {
//				model.setMode(GameModel.Mode.GAME_WON);
//			} else {
//			//show "game won" title and offer next level
//			model.setMode(GameModel.Mode.LEVEL_WON);
//			}
//		}
	}

//	private boolean checkIfAllEnemiesAreDead() {
//		boolean anyEnemy = false;
//		synchronized (model.getPlayers()) {
//			for (AbstractPlayerController absPlayer : model.getPlayers()) {
//				anyEnemy = anyEnemy || ((absPlayer instanceof EnemyController) && absPlayer.getModel().getHealth() > 0f);
//			}			
//		}
//		return !anyEnemy;
//	}

	private void updateCameraPosition(long diff) {
		if(!stickyCam) return;
		float diffInSec = (float)diff / 1000f;
		Vector3D eyeLocation = model.getEyeLocation();
		Vector3D lookAtLocation = model.getLookAtLocation();
		Vector3D camDirUp = model.getCameraDirectionUp();
		Vector3D.moveAVecToBVecByDiff(diffInSec, eyeLocation, model.DEFAULT_EYE_LOCATION);
		Vector3D.moveAVecToBVecByDiff(diffInSec, lookAtLocation, Vector3D.origin);
		Vector3D.moveAVecToBVecByDiff(diffInSec, camDirUp, Vector3D.Yaxis);
		
		/*		AbstractPlayerController player = model.getCurrentPlayer(); //.getPlayers().get(model.getCurrentPlayerObjectIndex());
		if(player==null) return;
		Vector3D pLoc = player.getModel().getLocation();
		Vector3D pDirUp = player.getDirectionUp();
		Vector3D pDirForward = player.getDirectionForeward();
		if(model.getMode() == GameModel.Mode.PLAYER_IS_MOVING) {
			Vector3D placeToSeePlayer = pLoc.add(pDirUp.multiply(.5f)).minus(pDirForward.multiply(.5f));
			Vector3D.moveAVecToBVecByDiff(diffInSec, eyeLocation, placeToSeePlayer);
			
			Vector3D.moveAVecToBVecByDiff(diffInSec, lookAtLocation, pLoc);

			Vector3D.moveAVecToBVecByDiff(diffInSec, camDirUp, pDirUp);
		} else if (model.getMode() == GameModel.Mode.PLAYER_IS_SHOOTING && player.getAimVector() != null) {
			Vector3D placeToSeePlayer = pLoc.add(pDirUp.multiply(.45f)).add(pDirForward.crossProduct(pDirUp).multiply(.45f));
			Vector3D.moveAVecToBVecByDiff(diffInSec, eyeLocation, placeToSeePlayer);
			
			Vector3D aimerLoc = model.getCurrentGunAimer().getModel().getLocation();
			Vector3D.moveAVecToBVecByDiff(diffInSec, lookAtLocation, aimerLoc);

			Vector3D.moveAVecToBVecByDiff(diffInSec, camDirUp, pDirUp);
		} else if (model.getMode() == GameModel.Mode.PROJECTILE_IS_FLYING) {
			AbstractWeapon proj = model.getCurrentProjectile();
			if(proj == null) return;
			Vector3D projLoc = new Vector3D(proj.getModel().getLocation());
			Vector3D dirUp = proj.getProjectileAimer().getDirectionUp();
			Vector3D placeToSeeProjectile = projLoc.add(dirUp.multiply(.25f))
													.add(proj.getProjectileAimer().getLocation().crossProduct(dirUp).multiply(.25f));
			
			Vector3D.moveAVecToBVecByDiff(diffInSec, eyeLocation, placeToSeeProjectile);
			
			Vector3D.moveAVecToBVecByDiff(diffInSec, lookAtLocation, projLoc);
		} else if (model.getMode() == GameModel.Mode.EXPLOSION) {
//			Vector3D expLoc = model.getCurrentExplosion().getModel().getLocation();
//			Vector3D verNormal = model.getCurrentExplosion().getExplosionVertex().getNormal();
//			Vector3D placeToSeeExplosion = expLoc.add(verNormal);
//			
//			Vector3D.moveAVecToBVecByDiff(diffInSec, eyeLocation, placeToSeeExplosion);
//			Vector3D.moveAVecToBVecByDiff(diffInSec, lookAtLocation, expLoc);
//			Vector3D.moveAVecToBVecByDiff(diffInSec, camDirUp, verNormal.crossProduct(placeToSeeExplosion.minus(expLoc)));
		} else if (model.getMode() == GameModel.Mode.PACKAGE_IS_FALLING) {
			PackageController currentPackage = model.getCurrentPackage();
			Vertex targetVertex = currentPackage.getTargetEdge().a;
			Vector3D vertexNormal = targetVertex.getNormal();
			Vector3D backDir = vertexNormal.crossProduct(Vector3D.Xaxis);
			Vector3D leftDir = backDir.crossProduct(vertexNormal);
			Vector3D placeToSeePackage = model.getTransformToMesh().transform(targetVertex.getVector3D()).minus(backDir.multiply(.2f)).add(vertexNormal.multiply(.2f));
			Vector3D vecFromCamToPackage = placeToSeePackage.minus(currentPackage.getModel().getLocation());
			
			Vector3D.moveAVecToBVecByDiff(diffInSec, eyeLocation, placeToSeePackage);
			Vector3D.moveAVecToBVecByDiff(diffInSec, lookAtLocation, currentPackage.getModel().getLocation());
			Vector3D.moveAVecToBVecByDiff(diffInSec, camDirUp, vecFromCamToPackage.crossProduct(leftDir));
		}*/
	}


	
	public void keyPressed(KeyEvent e) {
		if(e.character == 's') {
			stickyCam = !stickyCam;
			return;
		}
		if(e.character == 'w') {
			wireframe = !wireframe;
			return;
		}
		if(e.character == 'r' && stickyCam == false) {
			System.out.println("r");
			stickyCam = true;
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e1) {
				e1.printStackTrace();
			}
			stickyCam = false;
			return;
		}
		if(e.character == 't') {
		    Strip strip = new Strip();
		    runvasObjIdToStrip.put(1, strip);
			model.getGameobjects().add(strip);
			model.getDynamicObjects().add(strip);
		}
//		if (e.character == SWT.ESC) {
//			lastMode = model.getMode();
//			model.setMode(GameModel.Mode.RESTART_MENU);
//		}
		//other keys
//		if(model.getMode() == GameModel.Mode.PLAYER_IS_MOVING) {
//			AbstractPlayerController player = model.getCurrentPlayer(); //.getPlayers().get(model.getCurrentPlayerObjectIndex());
//			if(player == null) return;
//			player.handleKey(e,true);
//		} else if (model.getMode() == GameModel.Mode.PLAYER_IS_SHOOTING) {
//			model.getCurrentGunAimer().handleKey(e,true);
//		} else if (model.getMode() == GameModel.Mode.MAIN_MENU ||
//				model.getMode() == GameModel.Mode.HELP_SCREEN) {
//			this.menuController.handleKey(e,true);
//		} else if (model.getMode() == GameModel.Mode.LEVEL_WON) {
//			if(e.character == SWT.CR) {
//				//enter pressed - goto next level
//				resetLevel(model.getCurrentLevel() + 1);
//			}
//		} else if (model.getMode() == GameModel.Mode.GAME_OVER) {
//			if(e.character == SWT.CR) {
//				//enter pressed - goto next level
//				resetLevel(model.getCurrentLevel());
//			}
//		}
////		} else if(model.getMode() == GameModel.Mode.RESTART_MENU) {	
//			if (e.keyCode == SWT.CR) {
//				resetLevel(model.getCurrentLevel());
//			} else if (e.keyCode == SWT.SHIFT) {
//				System.out.println(lastMode);
//				model.setMode(lastMode);
//			}
//		}
	}

	/*private void resetLevel(int levelToCreate) {
		//clear current level
		synchronized (model.getGameobjects()) {
			model.getGameobjects().clear();
		}
		synchronized (model.getDynamicObjects()) {
			model.getDynamicObjects().clear();
		}
		synchronized (model.getPackages()) {
			model.getPackages().clear();
		}
		synchronized (model.getPlayers()) {
			model.setCurrentPlayerObjectIndex(-1);
			model.getPlayers().clear();
		}
		
		model.setWorld(null);
		
		//build next level
		model.setCurrentLevel(levelToCreate);
		LevelsController.buildLevel(model, model.getCurrentLevel());
		worldInit = false;
		
		model.setCurrentPlayerObjectIndex(0);
		setMode(GameModel.Mode.PLAYER_IS_MOVING);
	}*/

	public void keyReleased(KeyEvent e) {
//		if(model.getMode() == GameModel.Mode.PLAYER_IS_MOVING) {
//			AbstractPlayerController player = model.getPlayers().get(model.getCurrentPlayerObjectIndex());
//			if (player != null) player.handleKey(e,false);
//		}else if (model.getMode() == GameModel.Mode.PLAYER_IS_SHOOTING) {
//			model.getCurrentGunAimer().handleKey(e,false);
//		}
	}

	public void mouseClicked(java.awt.event.MouseEvent e) {}

	public void mouseEntered(java.awt.event.MouseEvent e) {}

	public void mouseExited(java.awt.event.MouseEvent e) {}

	public void mousePressed(java.awt.event.MouseEvent e) {
		String modifiersExText = MouseEvent.getModifiersExText(e.getModifiersEx());
		if(modifiersExText.equals("Button2")) {
		    model.setEyeLocation(new Vector3D(GameModel.DEFAULT_EYE_LOCATION));
		    model.setCameraRotation(new Vector3D(GameModel.DEFAULT_CAMERA_ROTATION));
		    return;
		}
		m_mouseDown = true;
		if (stickyCam == true) {
			stickyCam = false;
			moveBack = true;
		}
	}

	public void mouseReleased(java.awt.event.MouseEvent e) {
		m_laspPosY = m_lastPosX = -1;
		m_mouseDown = false;
		if (moveBack == true) {
			stickyCam = true;
			moveBack = false;
		}
	}

	public void mouseDragged(java.awt.event.MouseEvent e) {
		String modifiersExText = MouseEvent.getModifiersExText(e.getModifiersEx());
		Vector3D eyeLocation = model.getEyeLocation();
		if(modifiersExText.equals("Button1")) {
			if(m_lastPosX > 0 || m_laspPosY > 0) {
				float dx = e.getX() - m_lastPosX;
				float dy = e.getY() - m_laspPosY;
	
				Vector3D lookAtLocation = model.getLookAtLocation();
				Vector3D left = lookAtLocation.minus(eyeLocation).crossProduct(model.getCameraDirectionUp()).getNormalized();
				eyeLocation.setTo(eyeLocation.add(left.multiply(dx/-100f)));

				Vector3D up = model.getCameraDirectionUp(); //model.getLookAtLocation().minus(eyeLocation).crossProduct(model.getCameraDirectionUp());
				eyeLocation.setTo(eyeLocation.add(up.multiply(dy/100f)));
				model.setCameraDirectionUp(left.crossProduct(lookAtLocation.minus(eyeLocation)).getNormalized());

//				eyeLocation.z += dy/100f;
			}
		} else if (modifiersExText.equals("Button3")) {
			if(m_lastPosX > 0 || m_laspPosY > 0) {
				float dx = e.getX() - m_lastPosX;
//				float dy = e.getY() - m_laspPosY;
				
//				AbstractPlayerController player = model.getCurrentPlayer(); //.getPlayers().get(model.getCurrentPlayerObjectIndex());
//				if(player != null) {
					Vector3D fromEyeToPlayer = eyeLocation.minus(model.lookAtLocation);
					eyeLocation.x += fromEyeToPlayer.x * (dx / 100f);
					eyeLocation.y += fromEyeToPlayer.y * (dx / 100f);
					eyeLocation.z += fromEyeToPlayer.z * (dx / 100f);
//				}
			}
		}
		
		m_lastPosX = e.getX();
		m_laspPosY = e.getY();
	}

	public void mouseMoved(java.awt.event.MouseEvent e) {}

	public Vector3D getLookAtLocation() {
		return model.getLookAtLocation();
	}

	public void setLookAtLocation(Vector3D lookAtLocation) {
		model.setLookAtLocation(lookAtLocation);
	}

//	public GameModel.Mode getMode() {
//		return model.getMode();
//	}
//
//	public void setMode(GameModel.Mode mode) {
//		model.setMode(mode);
//	}
	public List<IController> getGameobjects() {
		return model.getGameobjects();
	}
//	public WorldMeshObject getWorld() {
//		return model.getWorld();
//	}
//	public PlayerObjectController getPlayer() {
//		return model.getPlayer();
//	}

	public GameModel getModel() {
		return model;
	}

	public void setModel(GameModel model) {
		this.model = model;
	}

	public Vector3D getCameraDirectionUp() {
		return model.getCameraDirectionUp();
	}

	public boolean isWireframe() {
		return wireframe;
	}

	public boolean isStickyCam() {
		return stickyCam;
	}

	public void init(GL gl) {
		renderer.init(gl);
		
		//Grass rectangle
	    Texture tex = null;
		try {
			tex = TextureIO.newTexture(new FileInputStream("grass.jpg"), true, "png");
		    tex.setTexParameteri(GL.GL_TEXTURE_MIN_FILTER, GL.GL_LINEAR);
		    tex.setTexParameteri(GL.GL_TEXTURE_MAG_FILTER, GL.GL_LINEAR);
		} catch (GLException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}	
	    
		final Texture fTex = tex;
	    DefaultController dc = new DefaultController() {
	    	@Override
	    	public void additionalRotation(GL gl) {
	    		super.additionalRotation(gl);
	    		fTex.enable();
	    		fTex.bind();
	    	}
	    	@Override
	    	public void additionalDraw(GL gl) {
	    		super.additionalDraw(gl);
	    		fTex.disable();
	    	}
	    };
	    dc.getModel().setLocation(Vector3D.origin);
	    ArrayList<Vertex> vs = dc.getModel().getVertices();
	    Vertex a = new Vertex(-1,-1,0);
	    a.setNormal(Vector3D.Zaxis);
		vs.add(a);
	    Vertex b = new Vertex(-1,1,0);
	    b.setNormal(Vector3D.Zaxis);
		vs.add(b);
	    Vertex c = new Vertex(1,1,0);
	    c.setNormal(Vector3D.Zaxis);
		vs.add(c);
	    Vertex d = new Vertex(1,-1,0);
	    d.setNormal(Vector3D.Zaxis);
		vs.add(d);
	    ArrayList<Face> fs = dc.getModel().getQuadFaces();
	    Face fc = Face.createQuadFace(a, b, c, d, new Vector3D(0,0,1), false);
	    TextureCoords tc = fTex.getImageTexCoords();
	    float texC[] = new float[] {tc.top(),tc.right(),tc.top(),tc.left(),tc.bottom(),tc.left(),tc.bottom(),tc.right()};
	    fc.setTexCoords(texC);
		fs.add(fc);
	    model.getGameobjects().add(dc);
	}
}
package org.geekcon.runvas;

import java.util.ArrayList;
import java.util.List;

import org.geekcon.runvas.utils.Vector3D;

public class GameModel {
//	private static final double CHANCE_FOR_PACKAGE = 0.8;
	public static final float CAMERA_SPEED = 6f;
	public static final Vector3D DEFAULT_CAMERA_ROTATION = new Vector3D(90f,0f,0f);
	public static final Vector3D DEFAULT_EYE_LOCATION = new Vector3D(0f,0f,2.5f);
	private List<IController> gameobjects = new ArrayList<IController>();
	private List<IController> dynamicObjects = new ArrayList<IController>();
//	private WorldMeshObject world;
//	private PlayerObjectController player;
	
//	GunAimer currentGunAimer;
//	AbstractWeapon currentProjectile;
//	ExplosionController currentExplosion;
//	PackageController currentPackage;
	
//	int currentPlayerObjectIndex;
//	ArrayList<AbstractPlayerController> players = new ArrayList<AbstractPlayerController>();
	
	Vector3D eyeLocation = new Vector3D(0f,0f,5f);
	Vector3D cameraRotation = Vector3D.origin;
	Vector3D lookAtLocation = Vector3D.origin;
	private Vector3D cameraDirectionUp = Vector3D.Yaxis;
	
//	Transform3D transformToMesh;
	
//	ArrayList<PackageController> packages = new ArrayList<PackageController>();
	
//	int currentLevel;

//	public enum Mode {
//		MAIN_MENU,
//		PLAYER_IS_MOVING, 
//		PLAYER_IS_SHOOTING, 
//		PROJECTILE_IS_FLYING,
//		EXPLOSION, 
//		PACKAGE_IS_FALLING, 
//		LEVEL_WON, 
//		GAME_OVER, 
//		HELP_SCREEN,
//		RESTART_MENU, 
//		GAME_WON
//		};
//	private Mode mode;

	public List<IController> getGameobjects() {
		return gameobjects;
	}

	public void setGameobjects(List<IController> gameobjects) {
		this.gameobjects = gameobjects;
	}

	public List<IController> getDynamicObjects() {
		return dynamicObjects;
	}

	public void setDynamicObjects(List<IController> dynamicObjects) {
		this.dynamicObjects = dynamicObjects;
	}

//	public WorldMeshObject getWorld() {
//		return world;
//	}
//
//	public void setWorld(WorldMeshObject world) {
//		this.world = world;
//	}

	public Vector3D getEyeLocation() {
		return eyeLocation;
	}

	public void setEyeLocation(Vector3D eyeLocation) {
		this.eyeLocation = eyeLocation;
	}

	public Vector3D getCameraRotation() {
		return cameraRotation;
	}

	public void setCameraRotation(Vector3D cameraRotation) {
		this.cameraRotation = cameraRotation;
	}

	public Vector3D getLookAtLocation() {
		return lookAtLocation;
	}

	public void setLookAtLocation(Vector3D lookAtLocation) {
		this.lookAtLocation = lookAtLocation;
	}

//	public Mode getMode() {
//		return mode;
//	}
//
//	public void setMode(Mode mode) {
//		this.mode = mode;
//	}
//
//	public GunAimer getCurrentGunAimer() {
//		return currentGunAimer;
//	}
//
//	public void setCurrentGunAimer(GunAimer currentGunAimer) {
//		this.currentGunAimer = currentGunAimer;
//	}

	public void removeObject(IController obj) {
		synchronized (gameobjects) {
			gameobjects.remove(obj);
		}
		synchronized (dynamicObjects) {
			dynamicObjects.remove(obj);
		}
	}

	public void cleanUp() {
	    synchronized (gameobjects) {
		    List<IController> dOCopy = new ArrayList<IController>(gameobjects);
			for (IController controller : dOCopy) {
				if(controller.shouldBeRemoved()) {
					gameobjects.remove(controller);
				}
			}
	    }
	    synchronized (dynamicObjects) {
		    List<IController> dOCopy = new ArrayList<IController>(dynamicObjects);
			for (IController controller : dOCopy) {
				if(controller.shouldBeRemoved()) {
					dynamicObjects.remove(controller);
				}
			}
		}
//	    synchronized (players) {
//	    	List<AbstractPlayerController> playersCopy = new ArrayList<AbstractPlayerController>(players);
//	    	boolean removed = false;
//	    	for (AbstractPlayerController abstractPlayerController : playersCopy) {
//				if(abstractPlayerController.shouldBeRemoved()) {
//					players.remove(abstractPlayerController);
//					removed = true;
//				}
//			}
//	    	if(removed) nextPlayer();
//		}
	}

//	public AbstractWeapon getCurrentProjectile() {
//		return currentProjectile;
//	}
//
//	public void setCurrentProjectile(AbstractWeapon currentProjectile) {
//		this.currentProjectile = currentProjectile;
//	}

	public Vector3D getCameraDirectionUp() {
		return cameraDirectionUp;
	}

	public void setCameraDirectionUp(Vector3D cameraDirectionUp) {
		this.cameraDirectionUp = cameraDirectionUp;
	}

//	public int getCurrentPlayerObjectIndex() {
//		return currentPlayerObjectIndex;
//	}
//
//	public void setCurrentPlayerObjectIndex(int currentPlayerObjectIndex) {
//		this.currentPlayerObjectIndex = currentPlayerObjectIndex;
//	}
//
//	public ArrayList<AbstractPlayerController> getPlayers() {
//		return players;
//	}
//
//	public void setPlayers(ArrayList<AbstractPlayerController> players) {
//		this.players = players;
//	}


//	public void nextPlayer() {
//		synchronized (players) {
//			this.currentPlayerObjectIndex = (this.currentPlayerObjectIndex + 1) % players.size();
//			//next player should have full moves
//			players.get(currentPlayerObjectIndex).setNumMovesDone(0);
//			for (AbstractPlayerController player: players) {
//				player.setPlayerWasHit(null);
//			}
//		}
//	}
//
//	public AbstractPlayerController getCurrentPlayer() {
//		synchronized (players) {
//			if(currentPlayerObjectIndex < players.size()) {
//				if(currentPlayerObjectIndex < 0) {
//					return null;
//				}
//				return players.get(currentPlayerObjectIndex);
//			} else {
//				throw new RuntimeException("current player index is not consistent with players list (idx " + currentPlayerObjectIndex + ", size " + players.size() + ")");
//			}
//		}
//	}
//
//	public ExplosionController getCurrentExplosion() {
//		return currentExplosion;
//	}
//
//	public void setCurrentExplosion(ExplosionController currentExplosion) {
//		this.currentExplosion = currentExplosion;
//	}
//
//	public void nextMode() {
////		System.out.print("nextMode(): " + getCurrentPlayer().getPlayerName() + " : " + mode);
//		if(mode==Mode.PLAYER_IS_MOVING)
//			mode = Mode.PLAYER_IS_SHOOTING;
//		else if (mode == Mode.PLAYER_IS_SHOOTING) {
//			mode = Mode.PROJECTILE_IS_FLYING;
//		} else if (mode == Mode.PROJECTILE_IS_FLYING) {
//			if(currentProjectile == null || (currentProjectile != null && currentProjectile instanceof LaserProjectileController)) {
//				randForPackageOrGotoMove();
//			} else {
//				mode = Mode.EXPLOSION;
//			} 
//		} else if (mode == Mode.EXPLOSION) {
//			randForPackageOrGotoMove();
//		} else if (mode == Mode.PACKAGE_IS_FALLING) {
//			mode = Mode.PLAYER_IS_MOVING;
//		}
////		System.out.println(", next:" + mode);
//	}
//
//	private void randForPackageOrGotoMove() {
//		double rand = Math.random();
//		if(rand < CHANCE_FOR_PACKAGE) {
//			mode = Mode.PLAYER_IS_MOVING;
//		} else {
//			createPackage();
//		}
//	}
//
//	private void createPackage() {
//		PackageController packageController = new PackageController();
//		packageController.loadFromFile("crate.obj", false);
//		packageController.getModel().setScale(.0001f);
//		packageController.getModel().setColor(new Vector3D(.2f,.2f,.2f));
//		packageController.getModel().setGlobalOrientation(new Vector3D(0f,0f,0f));
//
//		ArrayList<Vertex> wrldVertices = world.getModel().getVertices();
//		Vertex v = wrldVertices.get((int)(Math.random() * wrldVertices.size()));
//		Edge edge = v.vertex.half.edge;
//		edge.packageOnEdge = currentPackage;
//		packageController.setTargetEdge(edge);
//		
//		packageController.getModel().setDirectionUp(edge.a.getNormal());
//		packageController.getModel().setDirectionForeward(edge.a.getNormal().crossProduct(Vector3D.Xaxis));
//		Vector3D betweenVertices = transformToMesh.transform(v.getVector3D()).add(transformToMesh.transform(edge.b.getVector3D())).multiply(.5f);
//		packageController.getModel().setLocation(betweenVertices.add(edge.a.getNormal().multiply(2f)));
//		packageController.setObjectsAndModeHandler(this);
//		packageController.setTransformToMesh(transformToMesh);
//		packageController.setPackageHandler(this);
//		
//		if(Math.random() < 0.5) {
//			packageController.setPackageType(PackageController.PackageType.AMMO);
//		} else {
//			packageController.setPackageType(PackageController.PackageType.HEALTH);
//		}
//		
//		setCurrentPackage(packageController);
//		
//		synchronized (dynamicObjects) {
//			dynamicObjects.add(packageController);
//		}
//		synchronized (gameobjects) {
//			gameobjects.add(packageController);
//		}
//		
//		mode = Mode.PACKAGE_IS_FALLING;
//	}
//
//	public PackageController getCurrentPackage() {
//		return currentPackage;
//	}
//
//	public void setCurrentPackage(PackageController currentPackage) {
//		this.currentPackage = currentPackage;
//	}
//
//	public Transform3D getTransformToMesh() {
//		return transformToMesh;
//	}
//
//	public void setTransformToMesh(Transform3D transformToMesh) {
//		this.transformToMesh = transformToMesh;
//	}
//
//	public ArrayList<PackageController> getPackages() {
//		return packages;
//	}
//
//	public int getCurrentLevel() {
//		return currentLevel;
//	}
//
//	public void setCurrentLevel(int currentLevel) {
//		this.currentLevel = currentLevel;
//	}
//
//	public void returnToLastMode() {
//		// TODO Auto-generated method stub
//		
//	}
	
	
}

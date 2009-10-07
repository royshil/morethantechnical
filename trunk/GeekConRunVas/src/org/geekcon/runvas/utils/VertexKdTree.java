//package org.geekcon.runvas.utils;
//
////import com.savarese.spatial.GenericPoint;
////import com.savarese.spatial.KDTree;
////import com.savarese.spatial.RangeSearchTree;
//
//import edu.wlu.cs.levy.CG.KDTree;
//import edu.wlu.cs.levy.CG.KeyDuplicateException;
//import edu.wlu.cs.levy.CG.KeySizeException;
//import ex3.controller.WorldMeshObject;
//
//public class VertexKdTree {
////	private static RangeSearchTree<Float, GenericPoint<Float>, Vertex>  vertexKdTree = new KDTree<Float, GenericPoint<Float>, Vertex>();
//	private static KDTree<Vertex> vertexKdTree; // = new KDTree<Vertex>(3);
//	
////	public static RangeSearchTree<Float, GenericPoint<Float>, Vertex> getVertexKdTree() {
////		return vertexKdTree;
////	}
//	
//	public static KDTree<Vertex> getVertexKdTree() {
//		return vertexKdTree;
//	}
//	
//	public static void newKDTree() {
//		vertexKdTree = new KDTree<Vertex>(3);
//	}
//
////	public static void setVertexKdTree(
////			RangeSearchTree<Float, GenericPoint<Float>, Vertex> vertexKdTree) {
////		VertexKdTree.vertexKdTree = vertexKdTree;
////	}
//
//	public static void createVertexKdTree(WorldMeshObject world, Transform3D t) {
//		for (Vertex  vertex : world.getModel().getVertices()) {
//			Vector3D vertexLocationMesh = new Vector3D(vertex.x, vertex.y, vertex.z);
//			Vector3D vertexLocationWorld = t.transform(vertexLocationMesh);
////			vertexKdTree.put(new GenericPoint<Float>(vertexLocationWorld.x, vertexLocationWorld.y, vertexLocationWorld.z), vertex);
//			
//			
//			double[] location = {vertexLocationWorld.x, vertexLocationWorld.y, vertexLocationWorld.z};
//			try {
//				vertexKdTree.insert(location, vertex);
//			} catch (KeySizeException e) {
//				e.printStackTrace();
//			} catch (KeyDuplicateException e) {
//				e.printStackTrace();
//			}
//		}	
//	}
//
//}

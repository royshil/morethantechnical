package org.geekcon.runvas;

import javax.media.opengl.GL;

public abstract class AbstractController implements IController {
    String filename;
    
    public AbstractController() {
    }
    
    public void additionalDraw(GL gl) {     }

    public void loadFromFile(String filename, boolean calcHalfEdge) {}
    	/*        this.filename = filename; 
            BufferedReader br = null;
            try {
                    br = new BufferedReader( 
                            new InputStreamReader(resConfig.getStream(filename))//new FileInputStream(filename))
                            );
                    String line = null;
                    ArrayList<Vertex> vertices = getModel().getVertices();
                    ArrayList<Face> triFaces = getModel().getTriFaces();
                    ArrayList<Face> quadFaces = getModel().getQuadFaces();
                    ArrayList<Vector3D> normals = new ArrayList<Vector3D>();
                    while ((line = br.readLine()) != null) {        
                            if(line.startsWith("vn ")) {
                                    StringTokenizer tokenizer = new StringTokenizer(line," ");
                                    tokenizer.nextToken();//skip "v"
                                    float a = Float.parseFloat(tokenizer.nextToken());
                                    float b = Float.parseFloat(tokenizer.nextToken());
                                    float c = Float.parseFloat(tokenizer.nextToken());
                                    normals.add(new Vector3D(a,b,c));
                            }
                            if(line.startsWith("v ")) {
                                    //vertex
                                    StringTokenizer tokenizer = new StringTokenizer(line," ");
                                    tokenizer.nextToken();//skip "v"
                                    float a = Float.parseFloat(tokenizer.nextToken());
                                    float b = Float.parseFloat(tokenizer.nextToken());
                                    float c = Float.parseFloat(tokenizer.nextToken());
                                    vertices.add(new Vertex(a,b,c));
                            }
                            if(line.startsWith("f ")) {
//                                  if(line.contains("//")) {
//                                          line = line.replaceAll("(\\d+)//\\d+", "$1");
//                                  }
//                                  if(line.contains("/")) {
//                                          line = line.replaceAll("(\\d+)/\\d+", "$1");
//                                  }
                                    StringTokenizer tokenizer = new StringTokenizer(line," ");
                                    tokenizer.nextToken();//skip "f"
                                    String nextToken = tokenizer.nextToken();
                                    int aN = -1;
                                    if(nextToken.contains("//")) {
                                            String normal = nextToken.substring(nextToken.indexOf("//")+2);
                                            aN = Integer.parseInt(normal);
                                            nextToken = nextToken.substring(0, nextToken.indexOf("//"));
                                    }
                                    int a = Integer.parseInt(nextToken);
                                    nextToken = tokenizer.nextToken();
                                    int bN = -1;
                                    if(nextToken.contains("//")) {
                                            String normal = nextToken.substring(nextToken.indexOf("//")+2);
                                            bN = Integer.parseInt(normal);
                                            nextToken = nextToken.substring(0, nextToken.indexOf("//"));
                                    }
                                    int b = Integer.parseInt(nextToken);
                                    nextToken = tokenizer.nextToken();
                                    int cN = -1;
                                    if(nextToken.contains("//")) {
                                            String normal = nextToken.substring(nextToken.indexOf("//")+2);
                                            cN = Integer.parseInt(normal);
                                            nextToken = nextToken.substring(0, nextToken.indexOf("//"));
                                    }
                                    int c = Integer.parseInt(nextToken);
                                    Vertex vA = vertices.get(a-1);
                                    Vertex vB = vertices.get(b-1);
                                    Vertex vC = vertices.get(c-1);
                                    if(vA==null||vB==null||vC==null) continue;
                                    Vector3D vAvec = vA.getVector3D();
                                    Vector3D vBvec = vB.getVector3D();
                                    Vector3D vCvec = vC.getVector3D();
                                    
                                    Vector3D bMinusA = vBvec.minus(vAvec);
                                    Vector3D bMinusC = vBvec.minus(vCvec);
                                    
                                    if(aN >= 0) {
                                            vA.setNormal(normals.get(aN-1));
                                    } else if(normals.size() > a-1) {
                                            vA.setNormal(normals.get(a-1));
                                    } else {
                                            Vector3D cMinusA = vCvec.minus(vAvec);
                                            
                                            vA.setNormal((cMinusA).crossProduct(bMinusA).getNormalized().multiply(-1f));
                                    }
                                    if(bN >= 0) {
                                            vB.setNormal(normals.get(bN-1));
                                    } else if(normals.size() > b-1) {
                                            vB.setNormal(normals.get(b-1));
                                    } else {
                                            Vector3D aMinusB = vAvec.minus(vBvec);
                                            Vector3D cMinusB = vCvec.minus(vBvec);
                                            vB.setNormal((aMinusB).crossProduct(cMinusB).getNormalized().multiply(-1f));
                                    }
                                    if(cN >= 0) {
                                            vC.setNormal(normals.get(cN-1));
                                    } else if(normals.size() > c-1) {
                                            vC.setNormal(normals.get(c-1));
                                    } else {
                                            Vector3D aMinusC = vAvec.minus(vCvec);
                                            vC.setNormal((bMinusC).crossProduct(aMinusC).getNormalized().multiply(-1f));
                                    }
                                    
                                    if(tokenizer.hasMoreTokens()) {
                                            int d = Integer.parseInt(nextToken);
                                            Vertex vD = vertices.get(d-1);
                                            if(vD == null) continue;
                                            if(normals.size() > d-1) {
                                                    vD.setNormal(normals.get(d-1));
                                            } else {
                                                    Vector3D vDvec = vD.getVector3D();
                                                    vA.setNormal((vDvec.minus(vAvec)).crossProduct(bMinusA).getNormalized().multiply(-1f));
                                                    vD.setNormal((vCvec.minus(vDvec)).crossProduct(vAvec.minus(vDvec)).getNormalized().multiply(-1f));
                                                    vC.setNormal((bMinusC).crossProduct(vDvec.minus(vCvec)).getNormalized().multiply(-1f));
                                            }
                                            Vector3D N = vA.getNormal().add(
                                                                    vB.getNormal()).add(
                                                                    vC.getNormal()).add(
                                                                    vD.getNormal()).multiply(.25f);
                                            quadFaces.add(Face.createQuadFace(
                                                                    vA, 
                                                                    vB, 
                                                                    vC, 
                                                                    vD, 
                                                                    N.getNormalized(), 
                                                                    calcHalfEdge));
                                    } else {
                                            Vector3D N = vA.getNormal().add(
                                                            vB.getNormal()).add(
                                                            vC.getNormal()).multiply(.333f);
                                            triFaces.add(Face.createTriangleFace(
                                                                    vA, 
                                                                    vB, 
                                                                    vC, 
                                                                    N.getNormalized(),
                                                                    calcHalfEdge));
                                    }
                            }
                                    
                    }

                    br.close();
            } catch (FileNotFoundException e) {
                    e.printStackTrace();
            } catch (IOException e) {
                    e.printStackTrace();
            } finally {
                    
            }
    }*/

    public void advance(long timeElapsed) {}

    public boolean shouldBeRemoved() {return false;}

	@Override
	public void additionalRotation(GL gl) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public AbstractModel getModel() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public IRenderer getRenderer() {
		// TODO Auto-generated method stub
		return null;
	}
}

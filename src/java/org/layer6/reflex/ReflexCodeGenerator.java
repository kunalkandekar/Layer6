package net.layer6.reflex;

import java.lang.reflect.Field;
import java.lang.reflect.Modifier;
import java.lang.reflect.Method;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.List;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.File;
import java.io.IOException;

public class ReflexCodeGenerator {
	final static String strImports = "import net.layer6.Layer6;\nimport net.layer6.Layer6Msg;\nimport net.layer6.Layer6Exception;\n"; 
	final static String strFullPrefix = "\npublic class ";
	final static String strIncPrefix = "\nclass ";
	//use reflection to generate L6 serialization code
	
	int fieldIdConstantsStart = 0;
	boolean useShallowRefsForGets = true;
	boolean useShallowRefsForSets = true;
	boolean useStringFieldIds = false;
	boolean fullPrefix = true;
	
	public static String tabs = "    ";
	
	Map<String, ReflexCodeGenerator> subGenMap;
	List<String> packages;
	Class target;
	
	public ReflexCodeGenerator() {
		fieldIdConstantsStart = 0;
		useShallowRefsForGets = true;
		useShallowRefsForSets = true;
		fullPrefix = true;
		subGenMap = new HashMap<String, ReflexCodeGenerator>();
		packages = new ArrayList<String>(); 
	}
	
	public ReflexCodeGenerator(Class c) {
		this();
		target = c;
	}
	
	public void setFieldIdConstantsStart(int id) {
		fieldIdConstantsStart = id;
	}
	
	public void setUseShallowRefsForGets(boolean b) {
		useShallowRefsForGets = b;
	}
	
	public void setUseShallowRefsForSets(boolean b) {
		useShallowRefsForSets = b;
	}
	
	public void setUseStringsFieldIds(boolean b) {
		useStringFieldIds = b;
	}
	
	public void setFullPrefix(boolean b) {
		fullPrefix = b;
	}
	
	public void restrictToPackages(String [] packs) {
		for(String pack : packs) { 
			packages.add(pack);
		}
	}
	
	
	public static String getPackageName(Class cls) {
		String ret = null;
		if(cls.getPackage() != null) {
			ret = cls.getPackage().getName(); 
		}
		else {
			Package pack = Package.getPackage(cls.getCanonicalName());
			if(pack != null) {
				ret = pack.getName();
			}
			else {
				//get class name and derive package from that
				String cname = cls.getCanonicalName();
				int index = cname.lastIndexOf(".");
				if(index > 0) {
					ret = cname.substring(0, index);
				}
			}
		}
		return ret;
	}
	
	public boolean isPackageAllowed(Class c) {
		String p = getPackageName(c);
		if(p != null) {
			for(String pack : packages) { 
				if(p.startsWith(pack)) {
					return true;
				}
			}
		}
		return false;
	}
	
	static String incIndent(String indent) {
		return indent+tabs;
	}
	
	static String decIndent(String indent) {
		return indent.substring(0, indent.length() - tabs.length());
	}

	public String codegen() throws Exception {
		return codegen(target);
	}
	
	public String codegen(Object obj) throws Exception {
		Class cls = obj.getClass();
		//String name = c.getName();
		return codegen(cls);
	}
	
	public static String generateSerializerClassName(String cname) {
		return ("L6"+cname+"Serializer");
	}
	
	public String makeMethodName(String prefix, String fieldName) {
		String fieldNameCap = Character.toUpperCase(fieldName.charAt(0)) + fieldName.substring(1);
		return prefix + fieldNameCap;
	}
	
	public String findMethod(Class cls, String prefix, String fieldName) {
		Method [] methods = cls.getMethods();
		String strFname = makeMethodName(prefix, fieldName); 
		for(Method method : methods) {
			if(method.getName().equalsIgnoreCase(strFname)) {
				strFname = method.getName();
				return strFname;
			}
		}
		return null;
	}
	
	public String findSetter(Class cls, String fieldName) {
		String fieldNameCap = Character.toUpperCase(fieldName.charAt(0)) + fieldName.substring(1);
		return "set"+fieldNameCap;
	}
	
	public static boolean isPrimitive(String typeString) {
		return (("int".equals(typeString)) || ("double".equals(typeString)) || ("byte".equals(typeString)) || ("long".equals(typeString)) 
				|| ("float".equals(typeString)) || ("short".equalsIgnoreCase(typeString)) || ("boolean".equalsIgnoreCase(typeString)) );
	}
	
	public String codegen(Class cls) throws Exception {
		//put self in map to avoid unnecessary recursion
		if(cls == null) {
			return "";
		}
		if(packages.size() < 1) {
			//init to current target class package by default
			packages.add(getPackageName(cls));
		}
		//check package
		if( !isPackageAllowed( cls ) )  {
			//outside package scope
			return "";
		}
		/*
		if(subGenMap.get(cls.getCanonicalName()) != null) {
			System.out.println("\nalready have rcg for cls=[" +cls.getCanonicalName()+ "] ="+subGenMap.get(cls.getCanonicalName())+"...\n");
			return "";
		}
		*/
		String indent = "\n"+tabs;
		String cname = cls.getSimpleName();
		String fname = cname;	//cls.getCanonicalName();
		if(cname.endsWith("[]")) {
			//array
			cname = cname.substring(0, cname.length() - 2);
		}
		if(fname.endsWith("[]")) {		
			fname = fname.substring(0, fname.length() - 2);
		}
		
		String serClassName = generateSerializerClassName(cname);
		StringBuffer sbuf = new StringBuffer(4096);
		if(fullPrefix) {
			sbuf.append(strImports);
			sbuf.append("import ");
			sbuf.append(cls.getPackage().getName());
			sbuf.append(".*;\n");
			sbuf.append(strFullPrefix);
		}
		else {
			sbuf.append(strIncPrefix);
		}
		sbuf.append(serClassName);
		sbuf.append(" {	//begin class");
		sbuf.append(indent);
		
		//field id constants
		Field [] fields = cls.getDeclaredFields();
		System.out.println("fields for "+cls+" = "+java.util.Arrays.toString(fields));
		int start = fieldIdConstantsStart;
		HashMap<String, String> idFieldMap = new HashMap<String, String>();
		HashMap<String, String> fieldNameGetterMap = new HashMap<String, String>();
		HashMap<String, String> fieldNameSetterMap = new HashMap<String, String>();
		ArrayList <Field> l6fields = new ArrayList<Field>();
		
		int arrayStart = 100;
		for(Field field : fields) {
			int mod = field.getModifiers();
			String fieldName = field.getName();
			if( (((mod & Modifier.PUBLIC) > 0) || ((mod & Modifier.PROTECTED) > 0) || ((mod & Modifier.PRIVATE) > 0)) 
					&& ((mod & Modifier.FINAL) == 0)) {
				String strFieldIdConst = "L6_MSG_FIELD_ID_"+fieldName.toUpperCase();
				if(useStringFieldIds) {
					sbuf.append(indent).append("public static final String ").append(strFieldIdConst);
					sbuf.append(" = \"").append(fieldName).append("\";");
				}
				else {
					sbuf.append(indent).append("public static final int ").append(strFieldIdConst);
					sbuf.append(" = ").append(start++).append(";");
				}
				boolean isPrivate   = ((mod & Modifier.PRIVATE) > 0); 
				boolean isProtected = ((mod & Modifier.PROTECTED) > 0); 

				String fieldGetter = findMethod(cls, "get", fieldName);
				if((fieldGetter == null) 
				   && (field.getType().getCanonicalName().equals("boolean"))) {
                    fieldGetter = findMethod(cls, "is", fieldName);
				}
				if(fieldGetter == null) {
                    System.out.println("WARNING! Getter not found for "
                        +(isPrivate ? "private" : (isProtected ? "protected" : "public" )) 
                        +" field "+fieldName +" of type "+field.getType().getCanonicalName()+" in "+cname);
                    if(isPrivate || isProtected) {
				        fieldGetter = makeMethodName("get", fieldName); //make one up
                    }
				}
				fieldNameGetterMap.put(fieldName, fieldGetter);
				//System.out.println(fieldGetter+" for "+fieldName+" of type "+field.getType().getCanonicalName()+" in "+cname); 
				
				String fieldSetter = findMethod(cls, "set", fieldName);
				if(fieldSetter == null) {
                    System.out.println("WARNING! Setter not found for "
                        +(isPrivate ? "private" : (isProtected ? "protected" : "public" )) 
                        +" field "+fieldName +" of type "+field.getType().getCanonicalName()+" in "+cname);
                    if(isPrivate || isProtected) {
    				    fieldSetter = makeMethodName("set", fieldName);
                    }
				}
				fieldNameSetterMap.put(fieldName, fieldSetter);

				idFieldMap.put(fieldName, strFieldIdConst);
				l6fields.add(field);
			}
		}
		
		while(arrayStart < start) {
			start+=100;
		}
		
		//generate field id constants
		
		String instanceName = "obj"+cname;
		sbuf.append("\n").append(indent);
		sbuf.append("public static Layer6Msg toLayer6Msg( ").append(fname).append(" ").append(instanceName).append(" )");
		sbuf.append(" throws Layer6Exception {	//begin toLayer6Msg(obj) method");
		indent = incIndent(indent);
		sbuf.append(indent).append("Layer6Msg msg = new Layer6Msg();");
		sbuf.append(indent).append("toLayer6Msg( ").append(instanceName).append(", msg );");
		sbuf.append(indent).append("return msg;");
		indent = decIndent(indent);
		sbuf.append(indent).append("}	//end toLayer6Msg(obj) method\n\n");
		
		sbuf.append(indent).append("public static void toLayer6Msg( ").append(fname).append(" ").append(instanceName).append(", ");
			sbuf.append(" Layer6Msg msg )");
			sbuf.append(" throws Layer6Exception {	 //begin toLayer6Msg(obj, msg) method");
		indent = incIndent(indent);
		sbuf.append(indent).append("int start = 0;		//only used in case of non-primitive arrays");
		sbuf.append(indent).append("int len   = 0;		//only used in case of non-primitive arrays");
		
		StringBuffer rbuf = new StringBuffer(1024);

		//the real meat of the class
		for(Field field : l6fields) {
			String fieldName = field.getName();
			String fieldId = idFieldMap.get(fieldName);
			String getter = fieldNameGetterMap.get(fieldName);
			if(getter != null) {
				getter = (instanceName+"."+getter+"()");
			}
			else {
				getter = (instanceName+"."+fieldName);
			}
			
			Class ctype = field.getType();
			String typeString = ctype.getSimpleName();
			boolean isArray = false;
			boolean isBool = false;
			if(typeString.endsWith("[]")) {
				//array
				isArray = true;
				typeString = typeString.substring(0, typeString.length() - 2);
			}
			//System.out.println("\n checking type=["+typeString+"]");
			boolean isPrimitive = isPrimitive(typeString);	//ctype.isPrimitive();
			
			String setter = fieldNameSetterMap.get(fieldName);
			
			String nonPrimArray = "";
			if(isArray && !isPrimitive) {
				nonPrimArray= "array"+fieldName;
				rbuf.append(indent).append(typeString).append(" [] ").append(nonPrimArray).append(";");
			}
			else {
				if(setter != null) {
					rbuf.append(indent).append(instanceName).append(".").append(setter).append("( ");
				}
				else {
					rbuf.append(indent).append(instanceName).append(".").append(fieldName).append(" = ");
				}
				sbuf.append(indent).append("msg.set");
				rbuf.append("msg.get");
			}

			if(isPrimitive) {
				//check in order of most likely on the average
				if("int".equalsIgnoreCase(typeString) || ("Integer".equalsIgnoreCase(typeString)) ) {
					sbuf.append("Int");
					rbuf.append("Int");
				}
				else if("double".equalsIgnoreCase(typeString) || ("Double".equalsIgnoreCase(typeString)) ) {
					sbuf.append("Double");
					rbuf.append("Double");
				}
				else if("byte".equalsIgnoreCase(typeString) || ("Byte".equalsIgnoreCase(typeString)) ) {
					sbuf.append("Byte");
					rbuf.append("Byte");
				}
				else if("long".equalsIgnoreCase(typeString) || ("Long".equalsIgnoreCase(typeString)) ) {
					sbuf.append("Long");
					rbuf.append("Long");
				}
				else if("float".equalsIgnoreCase(typeString) || ("Float".equalsIgnoreCase(typeString)) ) {
					sbuf.append("Float");
					rbuf.append("Float");
				}
				else if("short".equalsIgnoreCase(typeString) || ("Short".equalsIgnoreCase(typeString)) ) {
					sbuf.append("Short");
					rbuf.append("Short");
				}
				else if("boolean".equalsIgnoreCase(typeString) || ("Boolean".equalsIgnoreCase(typeString)) ) {
					sbuf.append("Short");
					rbuf.append("Short");
					isBool = true;
				}
				else {
					//uh oh!
					throw new Exception("Unrecognized primitive: "+typeString);
				}
				if(isArray) {
					if(isBool) {
						throw new Exception("Unsupported primitive-array combination: boolean []");
					}
					sbuf.append("Array");
					rbuf.append("Array");
					if(useShallowRefsForSets) {
						sbuf.append("Ref");
					}
					if(useShallowRefsForGets) {
						rbuf.append("Ref");
					}
				}
				sbuf.append("( ").append(serClassName).append(".").append(fieldId).append(", ");	//field ID
				rbuf.append("( ").append(serClassName).append(".").append(fieldId);
				if(isArray) {
					sbuf.append(getter).append(", 0, ").append(getter).append(".length");
				}
				else {
					if(isBool) {
						//use ternary operator
						sbuf.append("( ").append(getter).append(" ? (short) 1 : (short)0 )");
						rbuf.append(") ").append(" != 0 ");
					}
					else {
						sbuf.append(getter);
						//rbuf.append(fieldId);
					}
				}
				sbuf.append(" );");
				if(!isBool) {
					rbuf.append(" )");
				}
			}
			else {
				String refFieldId = serClassName+"."+fieldId; 
				String arrFieldId = "arrayFieldId";
				if(isArray) {
					sbuf.append(indent).append("start = ").append(arrayStart).append(";");
					sbuf.append(indent).append("len = ").append(getter).append(".length;");
					sbuf.append(indent).append("msg.setInt( ").append(refFieldId).append(", start );");
					sbuf.append(indent).append("msg.setInt( start, len );");
					sbuf.append(indent).append("for(int itr=0; itr < len; itr++) {");

					//for getting, get len first, by getting start
					rbuf.append(indent).append("start = msg.getInt( ").append(refFieldId).append(" );");
					rbuf.append(indent).append("len = msg.getInt( start );");
					rbuf.append(indent).append(nonPrimArray).append(" = new ").append(typeString).append("[len];");
					rbuf.append(indent).append("for(int itr=0; itr < len; itr++) {");
					
					indent = incIndent(indent);
					sbuf.append(indent).append("int ").append(arrFieldId).append(" = start + itr;");
					sbuf.append(indent).append("msg.set");
					rbuf.append(indent).append("int ").append(arrFieldId).append(" = start + itr;");
				}

				if("String".equalsIgnoreCase(typeString)) {
					sbuf.append("String");
						if(isArray) {
							rbuf.append(indent).append(nonPrimArray).append("[itr] = msg.get");
						}					
						rbuf.append("String");
						if(useShallowRefsForSets) {
							sbuf.append("Ref");
						}
						if(useShallowRefsForGets) {
							rbuf.append("Ref");
						}
						sbuf.append("( ").append(refFieldId);	//field ID
						rbuf.append("( ").append(refFieldId);	//field ID
						
						sbuf.append(", ").append(getter);
						
						sbuf.append(" );");
						rbuf.append(" )");
				}
				else {
					//it's another object
					String subMsgName = "subMsg_"+fieldName;
					String canonicalName = ctype.getCanonicalName();
					if(canonicalName.endsWith("[]")) {
						canonicalName = canonicalName.substring(0, canonicalName.length() - 2);
					}
					if(subGenMap.get(canonicalName) == null) {
						//String pack = getPackageName(ctype);
						System.out.println("chk: "+packages+"] cls=" +canonicalName+ ", clspack=["+getPackageName(ctype)+"] packname="+"...\n");
						if( isPackageAllowed( ctype ) ) {
							//within package scope, proceed
							ReflexCodeGenerator rcg = new ReflexCodeGenerator(Class.forName(canonicalName));
							subGenMap.put(canonicalName, rcg);
							System.out.println("new rcg for ["+fieldName+"] cls=[" +ctype+ "] fld="+field+"...\n");
						}
						else {
							System.out.println("WARNING! rcg for ["+fieldName+"] cls=["+ctype+"] fld="+field+" not in "+packages+", skipping...\n");
						}
					}
					
					sbuf.append("Layer6Msg( ").append(refFieldId).append(", ");
						sbuf.append(ReflexCodeGenerator.generateSerializerClassName(typeString));
						sbuf.append(".toLayer6Msg( ").append(getter).append(" ) );");
						
					if(isArray) {
						rbuf.append(indent).append(nonPrimArray).append("[itr] = ");
					}
					rbuf.append(ReflexCodeGenerator.generateSerializerClassName(typeString) );
					rbuf.append(".fromLayer6Msg( msg.getLayer6Msg( ");
					rbuf.append(refFieldId).append(" )");
					if(!isArray) {
                        rbuf.append(" )");
					}
				}
				if(isArray) {
					rbuf.append(" );");
					indent = decIndent(indent);
					sbuf.append(indent).append("}");
					rbuf.append(indent).append("}");
					sbuf.append(indent).append("start += len;");
					if(setter != null) {
						rbuf.append(indent).append(instanceName).append(".").append(setter).append("( ");
						rbuf.append(nonPrimArray).append(");");
					}
					else {
						rbuf.append(indent).append(instanceName).append(".").append(fieldName).append(" = ").append(nonPrimArray).append(";");
					}
				}
			}
			if(setter != null) {
				rbuf.append(" );");
			}
			else {
			    rbuf.append(";");
			}
		}
		
		
		indent = decIndent(indent);
		sbuf.append(indent).append("}	//end toLayer6Msg(obj, msg) method \n\n");
		
		//get obj rom msg
		sbuf.append(indent).append("public static ").append(fname).append(" fromLayer6Msg( Layer6Msg msg )");
		sbuf.append(" throws Layer6Exception {	//begin fromLayer6Msg(msg) method");
		indent = incIndent(indent);
		sbuf.append(indent).append(cname).append(" ").append(instanceName).append(" = new ").append(cname).append("();");
		sbuf.append(indent).append("fromLayer6Msg( ").append(instanceName).append(", msg );");
		sbuf.append(indent).append("return ").append(instanceName).append(";");
		indent = decIndent(indent);
		sbuf.append(indent).append("}	//end fromLayer6Msg(obj) method\n\n");
		
		sbuf.append(indent).append("public static void fromLayer6Msg( ").append(fname).append(" ").append(instanceName).append(", ");
			sbuf.append(" Layer6Msg msg ) throws Layer6Exception {	 //begin fromLayer6Msg(obj, msg) method");
		indent = incIndent(indent);
		sbuf.append(indent).append("int start = 0;		//only used in case of non-primitive arrays");
		sbuf.append(indent).append("int len   = 0;		//only used in case of non-primitive arrays");
		sbuf.append(rbuf.toString());
		indent = decIndent(indent);
		sbuf.append(indent).append("}	//end fromLayer6Msg(obj, msg) method\n\n");
		
		sbuf.append("}//end class\n");
		//do sub classes
		Set<String> keyset = subGenMap.keySet();
		for(String key : keyset) {
			//use same settings as this one
			ReflexCodeGenerator rcg = subGenMap.get(key);
			rcg.setUseShallowRefsForGets(this.useShallowRefsForGets);
			rcg.setUseShallowRefsForSets(this.useShallowRefsForSets);
			rcg.setUseStringsFieldIds(this.useStringFieldIds);
			rcg.restrictToPackages((String[])this.packages.toArray(new String[0]));
			//no need to do imports
			rcg.setFullPrefix(false);
			sbuf.append(rcg.codegen());
		}
		/*
		subGenMap.put(cls.getCanonicalName(), this);
		System.out.println("\nnew rcg for cls=[" +cls+ "]...\n");
		*/

		return sbuf.toString();
	}
	
	public void toFile(String fileName, String className) throws Exception {
		BufferedWriter bw = new BufferedWriter(new FileWriter(new File(fileName), false));
		bw.write(this.codegen(Class.forName(className)));
		bw.close();
	}
	
	public static void main(String [] args) {
        if(args.length < 2) {
            System.out.println("\tUsage: java [-cp classpath] net.layer6.reflex.ReflexCodeGenerator <input-class> <ouput-source-filepath>");
            System.out.println("\n\tNote that <input-class> must be fully specified, and must be found in the class path.");
            System.out.println("\t   EG: java -cp layer6.jar net.layer6.reflex.ReflexCodeGenerator net.layer6.test.PlaybackInfo ./L6PlaybackInfoSerializer.java\n");
            System.exit(1);
        }
		ReflexCodeGenerator rcg = new ReflexCodeGenerator();
		try {
   			rcg.toFile(args[1], args[0]);
		}
		catch(Exception ex) {
			ex.printStackTrace();
		}
		
	}
}
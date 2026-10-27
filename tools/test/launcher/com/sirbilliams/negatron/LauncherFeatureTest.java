package com.sirbilliams.negatron;
import java.io.*;
import java.nio.file.*;
import java.util.zip.CRC32;
public final class LauncherFeatureTest {
    static void check(boolean test) { if (!test) throw new AssertionError(); }
    static void text(File f, String value) throws Exception { Files.write(f.toPath(), value.getBytes("UTF-8")); }
    public static void main(String[] args) throws Exception {
        ControlProfile c = new ControlProfile(); c.step("bind:0", 1);
        check(c.map[0] == 1 && c.map[1] == 0); check(new ControlProfile(c.csv(), 0,20,100,false).csv().equals(c.csv()));
        check(new ControlProfile("0,0,2,3,9,10,7,8",0,20,100,false).csv().equals(new ControlProfile().csv()));
        check(new ControlProfile("0,1,2,3,6,10,7,8",0,20,100,false).csv().equals(new ControlProfile().csv()));
        c.step("move_dead",-1);check(c.movement==35);c.step("camera_speed",-1);check(c.speed==75);
        c.step("invert_y",1);check(c.invert);
        File dir=Files.createTempDirectory("neg-launcher-").toFile();
        File save=new File(dir,"autosave.tsk"), meta=new File(dir,"autosave.tsk.preview"), bmp=new File(dir,"autosave.tsk.bmp");
        try(DataOutputStream out=new DataOutputStream(new FileOutputStream(save))) {
            pascal(out,"18:31:01");out.writeInt(0x4c563033);pascal(out,"Level3.gam");pascal(out,"autosave.tsk");out.writeFloat(100);out.writeShort(3);out.writeInt(123);
        }
        CRC32 crc=new CRC32();crc.update(Files.readAllBytes(save.toPath()));
        text(meta,"1 "+crc.getValue()+" 3661 1\n");text(bmp,"thumbnail placeholder for metadata test");
        SavePreviewInfo info=SavePreviewInfo.read(save);check(info.seconds==3661 && info.picture.equals(bmp));
        LauncherPages.Slot auto=LauncherPages.slots(dir)[0];check(auto.present && auto.trackedSeconds==3661 && auto.preview.equals(bmp));
        check(SavePreviewInfo.duration(3661).equals("1:01:01"));
        check(LauncherPages.main(auto,false).items.get(0).detail.length==5);
        text(save,"replaced save");check(SavePreviewInfo.read(save).seconds==-1 && SavePreviewInfo.read(save).picture==null);
        text(meta,"1 0 -1 1\n");check(SavePreviewInfo.read(save).seconds==-1);
        text(meta,"2 0 1 1\n");check(SavePreviewInfo.read(save).seconds==-1);
        text(meta,"truncated");check(SavePreviewInfo.read(save).seconds==-1);
        Files.delete(meta.toPath());check(SavePreviewInfo.read(save).seconds==-1);
        for(File f:dir.listFiles()) f.delete();dir.delete();
        System.out.println("Launcher control-profile, matching/stale previews and legacy save regressions passed");
    }
    static void pascal(DataOutputStream out,String s)throws Exception {byte[] b=s.getBytes("ISO-8859-1");out.writeByte(b.length);out.write(b);}
}

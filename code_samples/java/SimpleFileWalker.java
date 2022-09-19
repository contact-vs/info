import java.io.IOException;

public class SimpleFileWalker {
    public static void main(String[] args) {
        int nDirs = 0;
        int nFiles = 0;
        long totalSize = 0L;
        try (Stream<Path> paths = Files.walk(Paths.get(args[0]))) {
            Iterable< Path > iterable = paths::iterator;
            for ( Path s : iterable )
            {
                if( Files.isDirectory(s, LinkOption.NOFOLLOW_LINKS) ) {
                    ++nDirs;
                    System.out.println( "Directory N" + nDirs + " = " + s );
                }
                if( Files.isRegularFile(s, LinkOption.NOFOLLOW_LINKS) ) {
                    ++nFiles;
                    System.out.println( "File N" + nFiles+ " = " + s );
                    totalSize+=Files.size(s);
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        System.out.println( "-------------------------------" );
        System.out.println( "Total directories=" + nDirs );
        System.out.println( "Total files=" + nFiles );
        System.out.println( "Total size=" + totalSize );
    }
}

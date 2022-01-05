    <?php
    $username = "root";
    $pass = "SP1234SP";
    $host = "sp.myddns.me";
    $db_name = "kulcs";
    $con = mysqli_connect ($host, $username, $pass);
    $db = mysqli_select_db ( $con, $db_name );
    ?>
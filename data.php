    <?php
    include ('connection.php');
    $sql_insert = "INSERT INTO data (key1, key2, key3, tag) VALUES ('".$_GET["key1"]."', '".$_GET["key2"]."', '".$_GET["key3"]."', '".$_GET["tag"]."')";
    if(mysqli_query($con,$sql_insert))
    {
    echo "Done";
    mysqli_close($con);
    }
    else
    {
    echo "error is ".mysqli_error($con );
    }
    ?>
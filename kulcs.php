    <?php
    $url=$_SERVER['REQUEST_URI'];
    header("Refresh: 5; URL=$url"); // Refresh the webpage every 5 seconds
    ?>
    <html>
    <head>
    <title>Kulcs logger SPv1.0</title>
    <style type="text/css">
    .table_titles {
    padding-right: 20px;
    padding-left: 20px;
    color: #FF0;
    background-color: #00aaFF;
    }
    table {
    border-collapse: collapse;    
    width: 100%;
    }
    tr:nth-child(even){
        background-color: #f2f2f2;
    }
    tr:hover {
        background-color: #ada;
    }
    td {
    border: 1px solid #ddd;
    padding: 8px;
    }
    body { font-family: 'Courier New', monospace; }
    </style>
    </head>
    <body>
    <h1>Kulcs logger:</h1>
    <table>
    <tr>
    <td class="table_titles">ID</td>
    <td class="table_titles">IDŐBÉLYEG</td>
    <td class="table_titles">TEREM</td>
    <td class="table_titles">KULCS</td>
    <td class="table_titles">NÉV</td>
    <td class="table_titles">TAG</td>
    </tr>
    <?php
    include('connection.php');
    $result = mysqli_query($con,'SELECT data.id,data.time,data.key1,data.key2,names.name,data.tag FROM data INNER JOIN names ON data.tag = names.tag ORDER BY id DESC ');
    // Process every record
    $oddrow = true;
    while($row = mysqli_fetch_array($result))
    {
    if ($oddrow)
    {
    $css_class=' class="table_cells_odd"';
    }
    else
    {
    $css_class=' class="table_cells_even"';
    }
    $oddrow = !$oddrow; 
    echo "<tr>";
    echo "<td '.$css_class.'>" . $row['id'] . "</td>";
    echo "<td '.$css_class.'>" . $row['time'] . "</td>";
    echo "<td '.$css_class.'>" . $row['key1'] . "</td>";
    echo "<td '.$css_class.'>" . $row['key2'] . "</td>";
    echo "<td '.$css_class.'>" . $row['name'] . "</td>";
    echo "<td '.$css_class.'>" . $row['tag'] . "</td>";
    echo "</tr>"; 
    }
     
    // Close the connection
    mysqli_close($con);
    ?>
    </table>
    </body>
    </html>
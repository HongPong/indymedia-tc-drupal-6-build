<?php
// $Id: features-admin-components.tpl.php,v 1.1.2.1 2009/03/16 15:18:58 jmiccolis Exp $
?>
<div class='clear-block features-components'>
  <div class='column'>
    <div class='info'>
      <h3><?php print $info['name'] ?></h3>
      <div class='description'><?php print $info['description'] ?></div>
      <?php print $dependencies ?>
    </div>
  </div>
  <div class='column'>
    <div class='components'>
      <?php print $components ?>
    </div>
  </div>
</div>
#!/bin/sh

#perl -i -pe 's|[^\w]Item\[\s*(.*?)\s*\]->|getItem(\1)->|g' Build/Editor/EditorMercs.cc

# # -               if ( Item[ gusMercsNewItemIndex ].usItemClass == IC_KEY )
# # +               if ( getItem(gusMercsNewItemIndex)->getItemClass() == IC_KEY )
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])Item\[\s*(.*?)\s*\]->|\1getItem(\2)->|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])Item\s+\[\s*(.*?)\s*\]->|\1getItem(\2)->|g' '{}' \;


# find . -name '*.cc' -exec perl -i -pe 's|INVTYPE\s+const[&]\s+(\w+)\s*=\s*Item\[(.*)\];|const ItemModel * \1 = getItem(\2);|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|item[.]ubGraphicNum|item->getGraphicNum()|g' '{}' \;

# find . -name '*.cc' -exec perl -i -pe 's|([^\w])Item\[\s*(.*?)\s*\].usItemClass|\1getItem(\2)->getItemClass()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])Item\[\s*(.*?)\s*\].ubClassIndex|\1getItem(\2)->getClassIndex()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])Item\[\s*(.*?)\s*\].ubCursor|\1getItem(\2)->getCursor()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])Item\[\s*(.*?)\s*\].ubGraphicType|\1getItem(\2)->getGraphicType()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])Item\[\s*(.*?)\s*\].ubGraphicNum|\1getItem(\2)->getGraphicNum()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])Item\[\s*(.*?)\s*\].ubWeight|\1getItem(\2)->getWeight()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])Item\[\s*(.*?)\s*\].ubPerPocket|\1getItem(\2)->getPerPocket()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])Item\[\s*(.*?)\s*\].usPrice|\1getItem(\2)->getPrice()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])Item\[\s*(.*?)\s*\].ubCoolness|\1getItem(\2)->getCoolness()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])Item\[\s*(.*?)\s*\].bReliability|\1getItem(\2)->getReliability()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])Item\[\s*(.*?)\s*\].bRepairEase|\1getItem(\2)->getRepairEase()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])Item\[\s*(.*?)\s*\].fFlags|\1getItem(\2)->getFlags()|g' '{}' \;

# find . -name '*.cc' -exec perl -i -pe 's|item-->|item->|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|tem-->|tem->|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|item->ubClassIndex|item->getClassIndex()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|item.ubClassIndex|item->getClassIndex()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|ammo->ubClassIndex|ammo->getClassIndex()|g' '{}' \;

# find . -name '*.cc' -exec perl -i -pe 's|const\s+INVTYPE[*]\s+const\s+(\w+)\s*=\s[&]Item\[(.*)\];|const ItemModel * \1 = getItem(\2);|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])Item\[\s*(.*?)\s*\]|\1getItem(\2)|g' '{}' \;

# find . -name '*.cc' -exec perl -i -pe 's|item([\w]+).usPrice|item\1->getPrice()|g' '{}' \;

# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item(\w+).usItemClass|\1item\2->getItemClass()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item(\w+).ubClassIndex|\1item\2->getClassIndex()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item(\w+).ubCursor|\1item\2->getCursor()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item(\w+).ubGraphicType|\1item\2->getGraphicType()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item(\w+).ubGraphicNum|\1item\2->getGraphicNum()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item(\w+).ubWeight|\1item\2->getWeight()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item(\w+).ubPerPocket|\1item\2->getPerPocket()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item(\w+).usPrice|\1item\2->getPrice()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item(\w+).ubCoolness|\1item\2->getCoolness()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item(\w+).bReliability|\1item\2->getReliability()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item(\w+).bRepairEase|\1item\2->getRepairEase()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item(\w+).fFlags|\1item\2->getFlags()|g' '{}' \;

# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item->usItemClass|\1item\2->getItemClass()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item->ubClassIndex|\1item\2->getClassIndex()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item->ubCursor|\1item\2->getCursor()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item->ubGraphicType|\1item\2->getGraphicType()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item->ubGraphicNum|\1item\2->getGraphicNum()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item->ubWeight|\1item\2->getWeight()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item->ubPerPocket|\1item\2->getPerPocket()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item->usPrice|\1item\2->getPrice()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item->ubCoolness|\1item\2->getCoolness()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item->bReliability|\1item\2->getReliability()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item->bRepairEase|\1item\2->getRepairEase()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|([^\w])item->fFlags|\1item\2->getFlags()|g' '{}' \;

# find . -name '*.cc' -exec perl -i -pe 's|tem->usItemClass|tem\1->getItemClass()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|tem->ubClassIndex|tem\1->getClassIndex()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|tem->ubCursor|tem\1->getCursor()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|tem->ubGraphicType|tem\1->getGraphicType()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|tem->ubGraphicNum|tem\1->getGraphicNum()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|tem->ubWeight|tem\1->getWeight()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|tem->ubPerPocket|tem\1->getPerPocket()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|tem->usPrice|tem\1->getPrice()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|tem->ubCoolness|tem\1->getCoolness()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|tem->bReliability|tem\1->getReliability()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|tem->bRepairEase|tem\1->getRepairEase()|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|tem->fFlags|tem\1->getFlags()|g' '{}' \;

# find . -name '*.cc' -exec perl -i -pe 's|getItem\(|GCM->getItem\(|g' '{}' \;

# find . -name '*.cc' -exec perl -i -pe 's|GCM->getMagazine\((.*?)->getClassIndex\(\)\)->|\1->asAmmo\(\)->|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|GCM->getWeapon\((.*?)->getClassIndex\(\)\)->|\1->asWeapon\(\)->|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_AMMO|->isAmmo\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_ARMOUR|->isArmour\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_BLADE|->isBlade\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_BOMB|->isBomb\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_FACE|->isFace\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_GRENADE|->isGrenade\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_GUN|->isGun\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_KEY|->isKey\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_KIT|->isKit\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_LAUNCHER|->isLauncher\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_MEDKIT|->isMedkit\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_MISC|->isMisc\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_MONEY|->isMoney\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_PUNCH|->isPunch\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_TENTACLES|->isTentacles\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_THROWING_KNIFE|->isThrowingKnife\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_THROWN|->isThrown\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_WEAPON|->isWeapon\(\)|g' '{}' \;
# find . -name '*.cc' -exec perl -i -pe 's|->getItemClass\(\)\s*[&]\s*IC_EXPLOSV|->isExplosive\(\)|g' '{}' \;

find . -name '*.cc' -exec perl -i -pe 's|->getFlags\(\)\s*&\s*ITEM_TWO_HANDED|->isTwoHanded\(\)|g' '{}' \;

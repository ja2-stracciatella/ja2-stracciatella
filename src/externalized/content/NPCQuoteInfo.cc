#include "ContentManager.h"
#include "enums.h"
#include "Interface_Dialogue.h"
#include "ItemModel.h"
#include "Json.h"
#include "content/NPCQuoteInfo.h"


std::unique_ptr<NPCQuoteInfo const []> NPCQuoteInfo::deserialize(const JsonValue& json, const ContentManager * contentManager)
{
	auto reader = json.toObject();
	auto jsonRecords = reader["records"].toVec();
	auto buf = std::make_unique<NPCQuoteInfo[]>(NUM_NPC_QUOTE_RECORDS);
	for (auto const& jsonValue : jsonRecords)
	{
		auto const jsonRec = jsonValue.toObject();
		auto const recordIndex = jsonRec.GetUInt("index");
		NPCQuoteInfo* rec = &buf[recordIndex];
		rec->ubIdentifier = recordIndex;

		rec->fFlags = 0;
		if (jsonRec.getOptionalBool("alreadySaid")) rec->fFlags |= QUOTE_FLAG_SAID;
		if (jsonRec.getOptionalBool("eraseOnceSaid")) rec->fFlags |= QUOTE_FLAG_ERASE_ONCE_SAID;
		if (jsonRec.getOptionalBool("sayOncePerConvo")) rec->fFlags |= QUOTE_FLAG_SAY_ONCE_PER_CONVO;

		if (jsonRec.getOptionalBool("requiredAnyItem")) rec->sRequiredItem = ACCEPT_ANY_ITEM;
		else if (jsonRec.getOptionalBool("requiredAnyRifle")) rec->sRequiredItem = ANY_RIFLE;
		else if (jsonRec.has("requiredItem"))
		{
			ST::string jsonItem = jsonRec.GetString("requiredItem");
			const ItemModel* item = contentManager->getItemByName(jsonItem);
			rec->sRequiredItem = item->getItemIndex();
		}
		else rec->sRequiredGridno = -jsonRec.getOptionalInt("requiredGridNo");

		rec->usFactMustBeTrue = Internals::getFactEnumFromString(jsonRec.getOptionalString("factMustBeTrue"));
		rec->usFactMustBeFalse = Internals::getFactEnumFromString(jsonRec.getOptionalString("factMustBeFalse"));

		if (jsonRec.has("quest"))
		{
			auto q = jsonRec["quest"].toObject();
			uint8_t qCode = Internals::getQuestEnumFromString(q.GetString("name"));
			ST::string qStatus = q.GetString("status");
			if (qStatus == "DONE") rec->ubQuest = qCode + QUEST_DONE_NUM;
			else if (qStatus == "NOTSTARTED") rec->ubQuest = qCode + QUEST_NOT_STARTED_NUM;
			else if (qStatus == "INPROGRESS") rec->ubQuest = qCode;
		}
		else rec->ubQuest = Internals::getQuestEnumFromString("NO_QUEST");

		rec->ubFirstDay = jsonRec.getOptionalUInt("firstDay");
		rec->ubLastDay = jsonRec.getOptionalUInt("lastDay", IRRELEVANT);
		rec->ubApproachRequired = Internals::getApproachEnumFromString(jsonRec.getOptionalString("requiredApproach"));
		rec->ubOpinionRequired = jsonRec.getOptionalUInt("requiredOpinion");
		rec->ubQuoteNum = jsonRec.getOptionalUInt("quoteNum", IRRELEVANT);
		rec->ubNumQuotes = jsonRec.getOptionalUInt("numQuotes", IRRELEVANT);
		rec->ubStartQuest = Internals::getQuestEnumFromString(jsonRec.getOptionalString("startQuest"));
		rec->ubEndQuest = Internals::getQuestEnumFromString(jsonRec.getOptionalString("endQuest"));

		if (jsonRec.getOptionalBool("triggerClosestMerc")) rec->ubTriggerNPC = 0;
		else if (jsonRec.getOptionalBool("triggerSelf")) rec->ubTriggerNPC = 1;
		else if (jsonRec.has("triggerNPC"))
		{
			ST::string jsonProfileName = jsonRec.GetString("triggerNPC");
			rec->ubTriggerNPC = contentManager->getMercProfileInfoByName(jsonProfileName)->profileID;
		}
		else rec->ubTriggerNPC = IRRELEVANT;

		rec->ubTriggerNPCRec = jsonRec.getOptionalUInt("triggerRecord", IRRELEVANT);
		rec->usSetFactTrue = Internals::getFactEnumFromString(jsonRec.getOptionalString("setFactTrue"));

		if (jsonRec.has("giftItem"))
		{
			ST::string jsonItem = jsonRec.GetString("giftItem");
			if (jsonItem != "65535")
			{
				const ItemModel* item = contentManager->getItemByName(jsonItem);
				rec->usGiftItem = item->getItemIndex();
			}
			else rec->usGiftItem = 65535;
		}
		else if (jsonRec.has("userInterface"))
		{
			ST::string jsonStr = jsonRec.GetString("userInterface");
			if (jsonStr == "TURN_UI_ON") rec->usGiftItem = TURN_UI_ON;
			else if (jsonStr == "TURN_UI_OFF") rec->usGiftItem = TURN_UI_OFF;
		}

		rec->usGoToGridno = jsonRec.getOptionalUInt("goToGridno", NO_MOVE);

		if (jsonRec.has("actionData"))
		{
			auto a = jsonRec["actionData"].toObject();
			int16_t result = 0;
			if (a.has("turnToFace"))
			{
				ST::string jsonProfileName = a.GetString("turnToFace");
				uint8_t jsonProfileId = contentManager->getMercProfileInfoByName(jsonProfileName)->profileID;
				result = NPC_ACTION_TURN_TO_FACE_NEAREST_MERC + jsonProfileId;
			}
			else if (a.has("code")) result = Internals::getNPCActionEnumFromString(a.GetString("code"));
			if (a.getOptionalBool("doFirst")) result = -result;
			rec->sActionData = result;
		}
		else rec->sActionData = NPC_ACTION_NONE;
	}
	return buf;
}

JsonValue NPCQuoteInfo::serialize(const ContentManager* contentManager) const
{
	JsonObject obj;
	// triple digits prepended to field names is a workaround to sort them in convenient order instead of alphabetically
	obj.set("000index", this->ubIdentifier);
	if (this->fFlags != 0) {
		if (this->fFlags & QUOTE_FLAG_SAID) obj.set("001alreadySaid", true);
		if (this->fFlags & QUOTE_FLAG_ERASE_ONCE_SAID) obj.set("002eraseOnceSaid", true);
		if (this->fFlags & QUOTE_FLAG_SAY_ONCE_PER_CONVO) obj.set("003sayOncePerConvo", true);
	}

	if (this->sRequiredItem != 0) {
		if (this->sRequiredItem > 0) {
			const ItemModel* item = contentManager->getItem(this->sRequiredItem);
			if (item == nullptr) {
				if (this->sRequiredItem == ACCEPT_ANY_ITEM) obj.set("004requiredAnyItem", true);
				else if (this->sRequiredItem == ANY_RIFLE) obj.set("005requiredAnyRifle", true);
				else obj.set("006requiredItem", ST::format("{}",this->sRequiredItem));
			}
			else obj.set("007requiredItem", item->getInternalName());
		}
		else obj.set("008requiredGridNo", -this->sRequiredGridno);
	}

	if (this->usFactMustBeTrue != FACT_NONE) {
		obj.set("009factMustBeTrue", Internals::getFactName(static_cast<Fact>(this->usFactMustBeTrue)));
	}
	if (this->usFactMustBeFalse != FACT_NONE) {
		obj.set("010factMustBeFalse", Internals::getFactName(static_cast<Fact>(this->usFactMustBeFalse)));
	}

	if (this->ubQuest != IRRELEVANT) {
		JsonObject quest;
		if (this->ubQuest > QUEST_DONE_NUM) {
			quest.set("011index", this->ubQuest - QUEST_DONE_NUM);
			quest.set("012status", "DONE");
		}
		else if (this->ubQuest > QUEST_NOT_STARTED_NUM) {
			quest.set("013index", this->ubQuest - QUEST_NOT_STARTED_NUM);
			quest.set("014status", "NOTSTARTED");
		}
		else {
			quest.set("015index", this->ubQuest);
			quest.set("016status", "INPROGRESS");
		}
		obj.set("017quest", quest.toValue());
	}

	if (this->ubFirstDay != 0) {
		obj.set("018firstDay", this->ubFirstDay);
	}
	if (this->ubLastDay != IRRELEVANT) {
		obj.set("019lastDay", this->ubLastDay);
	}
	if (this->ubApproachRequired != APPROACH_NONE) {
		obj.set("020requiredApproach", Internals::getApproachName(static_cast<Approach>(this->ubApproachRequired)));
	}
	if (this->ubOpinionRequired != 0) {
		obj.set("021requiredOpinion", this->ubOpinionRequired);
	}

	if (this->ubQuoteNum != IRRELEVANT) {
		obj.set("022quoteNum", this->ubQuoteNum);
	}
	if (this->ubNumQuotes != IRRELEVANT) {
		obj.set("023numQuotes", this->ubNumQuotes);
	}

	if (this->ubStartQuest != IRRELEVANT) {
		obj.set("024startQuest", this->ubStartQuest);
	}
	if (this->ubEndQuest != IRRELEVANT) {
		obj.set("025endQuest", this->ubEndQuest);
	}

	if (this->ubTriggerNPC != IRRELEVANT) {
		// trigger closest merc who can see NPC
		if (this->ubTriggerNPC == 0) obj.set("026triggerClosestMerc", true);
		else if (this->ubTriggerNPC == 1) obj.set("027triggerSelf", true);
		else obj.set("028triggerNPC", (contentManager->getMercProfileInfo(this->ubTriggerNPC))->internalName);
	}
	if (this->ubTriggerNPCRec != IRRELEVANT) {
		obj.set("029triggerRecord", this->ubTriggerNPCRec);
	}
	if (this->usSetFactTrue != FACT_NONE) {
		obj.set("030setFactTrue", Internals::getFactName(static_cast<Fact>(this->usSetFactTrue)));
	}

	if (this->usGiftItem != 0) {
		const ItemModel* item = contentManager->getItem(this->usGiftItem);
		if (item == nullptr) {
			switch (this->usGiftItem)
			{
			case TURN_UI_OFF:
				obj.set("031userInterface", "TURN_UI_OFF");
				break;
			case TURN_UI_ON:
				obj.set("031userInterface", "TURN_UI_ON");
				break;
			case SPECIAL_TURN_UI_OFF:
				obj.set("032userInterface", "SPECIAL_TURN_UI_OFF");
				break;
			case SPECIAL_TURN_UI_ON:
				obj.set("033userInterface", "SPECIAL_TURN_UI_ON");
				break;
			default:
				obj.set("034userInterface", ST::format("{}", this->usGiftItem));
				break;
			}
		}
		else {
			obj.set("035giftItem", item->getInternalName());
		}
	}

	if (this->usGoToGridno != NO_MOVE) {
		obj.set("036goToGridno", this->usGoToGridno);
	}

	if (this->sActionData != NPC_ACTION_NONE) {
		JsonObject actionData;
		if (abs(this->sActionData) > NPC_ACTION_TURN_TO_FACE_NEAREST_MERC && abs(this->sActionData) < NPC_ACTION_LAST_TURN_TO_FACE_PROFILE) {
			actionData.set("037turnToFace", (contentManager->getMercProfileInfo(abs(this->sActionData) - NPC_ACTION_TURN_TO_FACE_NEAREST_MERC))->internalName);
		}
		else actionData.set("038action", Internals::getNPCActionName(static_cast<NPCAction>(abs(this->sActionData))));
		if (this->sActionData < 0) {
			actionData.set("039doFirst", true);
		}
		obj.set("040actionData", actionData.toValue());
	}

	return obj.toValue();
}

cost = {1,1,1,1}
icon = "frozen_land"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Grand Temple"
	else
		return "Great Temple"
	end
end

function getDescription()
	if language == "french" then
		return "Double la capacité en mana d'un temple."
	else
		return "Double temple produced mana"
	end
end

function onCast()
	selectTarget("temple", "")
end

function onResolve(temple)
	attachToTemple(temple)
end

function getMod_amount(val)
	return val * 2
end

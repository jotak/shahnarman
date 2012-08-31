icon = "mana"

-- life, law, death, chaos
mana = {0,0,0,0}

function getName()
	if language == "french" then
		return "Producteur de mana"
	else
		return "Mana productor"
	end
end

function getDescription()
	descr = ""
	if tonumber(mana[1]) > 0 then
		descr = descr..mana[1].." ¤, "
	end
	if tonumber(mana[2]) > 0 then
		descr = descr..mana[2].." |, "
	end
	if tonumber(mana[3]) > 0 then
		descr = descr..mana[3].." µ, "
	end
	if tonumber(mana[4]) > 0 then
		descr = descr..mana[4].." §, "
	end
	if language == "french" then
		if descr == "" then
			return "ne produit rien."
		else
			return "produit "..descr:sub(1, -3)
		end
	else
		if descr == "" then
			return "doesn't product anything."
		else
			return "products "..descr:sub(1, -3)
		end
	end
end

function init(str)
	i = 0
	for val in str:gmatch("[0-9]+")
	do
		i = i + 1
		mana[i] = 0+val
	end
	life = "0"
	law = "0"
	death = "0"
	chaos = "0"
	if mana[1] > 0 then
		life = "1"
	end
	if mana[2] > 0 then
		law = "1"
	end
	if mana[3] > 0 then
		death = "1"
	end
	if mana[4] > 0 then
		chaos = "1"
	end
	icon = "mana_"..life..law..death..chaos
end

function onNewUnitTurn(unitids)
	objtype, player, unit = split(unitids, " ")
	x, y = getObjectPosition("unit", player, unit)
	produceMana(player, x, y, mana[1], mana[2], mana[3], mana[4])
end

isMergeable = 1
function merge(params)
	i = 0
	for val in params:gmatch("[0-9]+")
	do
		i = i + 1
		mana[i] = mana[i] + val
	end
	return ""..mana[1].." "..mana[2].." "..mana[3].." "..mana[4]
end
